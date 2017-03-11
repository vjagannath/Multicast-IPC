#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/ds.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include "kernel/proc.h"
#include <stdlib.h>
#include "mipc.h"

// To Enqueue an message
int Enqueue(mipc_queue* dest_queue, message x) 
{
	
	int ret = OK;
	if(dest_queue->qsize < MAX_MIPC_QUEUE_LENGTH)
	{
		mipc_queueentry* temp = (mipc_queueentry*)malloc(sizeof(mipc_queueentry));
		temp->data = x;
		temp->readCount = 0;
		temp->next = NULL;
		
		if(dest_queue->front == NULL && dest_queue->rear == NULL)
		{
			dest_queue->front = dest_queue->rear = temp;
		}
		else
		{
			dest_queue->rear->next = temp;
			dest_queue->rear = temp;
		}
		dest_queue->qsize += 1;
		
		ret = OK;
		
	}
	else
	{
		ret = ENOBUFS;
	}
	
	return ret;
}

// To Dequeue an message.
void Dequeue(mipc_queue* target_queue) 
{
	mipc_queueentry* temp = target_queue->front;
	
	if(target_queue->front != NULL) 
	{
		if(target_queue->front == target_queue->rear) 
		{
			target_queue->front = target_queue->rear = NULL;
		}
		else 
		{
			target_queue->front = target_queue->front->next;
		}
		free(temp);
		target_queue->qsize -= 1;
	}
	
}

//To get the next message
mipc_queueentry* GetNext(mipc_queue* target_queue, mipc_queueentry* previous_entry) {
	
	mipc_queueentry* next_entry = NULL;
	
	if(previous_entry == NULL)
	{
		if(target_queue->front != NULL)
		{
			next_entry = target_queue->front;
		}
	}
	else
	{
		if(previous_entry->next != NULL)
		{
			next_entry = previous_entry->next;
		}
	}
	
	return next_entry;	
}

/*===========================================================================*
 *				do_msend			     *
 *===========================================================================*/
int
do_msend(void)
{
	mipc_queue* dest_queue = NULL;
	multicast_group *group = NULL;
	unsigned short gid = m_in.m1_i1;
	int r = OK;
	message msg;
	
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(gid == multiCastGroup[i].multiCastGroupID)
		{			
			group = &multiCastGroup[i];
			break;
		}
	}
	
	if(group != NULL)
	{
		dest_queue = &(group->msgQueue);
		
		r = sys_datacopy(who_e, (vir_bytes)m_in.m1_p1, SELF,
			(vir_bytes)&msg, sizeof(message));
		
			
		if (r == OK)
		{
			r = Enqueue(dest_queue, msg);
		}
	}
	
	return r;
}

/*===========================================================================*
 *				do_mreceive			     *
 *===========================================================================*/
int
do_mreceive(void)
{
	mipc_queue* target_queue = NULL;
	multicast_group *group = NULL;
	unsigned short gid = m_in.m1_i1;
	int r = OK;
	mipc_queueentry* msg = NULL;
	int processBelongsToGroup = 0;
	int processIndexInGroup = 0;
	
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(gid == multiCastGroup[i].multiCastGroupID)
		{			
			group = &multiCastGroup[i];
			break;
		}
	}
	
	if(group != NULL)
	{
		target_queue = &(group->msgQueue);
		
		for(int i = 0; i < group->currentMemberCount; i++)
		{
			if(who_e == group->groupMembers[i].epId)
			{
				processBelongsToGroup = 1;
				processIndexInGroup = i;
				break;
			}
		}
		
		if(processBelongsToGroup)
		{
			while(msg == NULL)
				msg = GetNext(target_queue, group->groupMembers[processIndexInGroup].prevRead);
			
			if(msg != NULL)
			{
				r = sys_datacopy(SELF, (vir_bytes)&(msg->data), who_e, (vir_bytes)m_in.m1_p1, sizeof(message));
			
				if (r == OK)
				{
					msg->readCount += 1;
					group->groupMembers[processIndexInGroup].prevRead = msg;
				}
			}
			
			if(target_queue->front->readCount >= group->currentMemberCount)
			{
				for(int i = 0; i < group->currentMemberCount; i++)
				{
					if(target_queue->front == group->groupMembers[i].prevRead)
					{
						group->groupMembers[i].prevRead = NULL;
					}
				}
				Dequeue(target_queue);	
			}
				
		}
		else
		{
			r = EACCES;
		}
	}
	else
	{
		r = EINVAL;
	}

	return r;
}

/*===========================================================================*
 *				do_opengroup			     *
 *===========================================================================*/
int
do_opengroup(void)
{
	/* 	1. Get the pid of the caller receiver process
	 *	2. Read the input group id passed by the receiver process - m_in.m1_i1
	 *	3. Check if the group with the input id is already present
	 *	4. If group is not present, create a new group with the group id and add the process to the process list of the group
	 *	5. If group is present, check if the current process is already a member of the process,
	 *		6. If already a member of the group, return with a message "Process is already a member of this group"
	 *		7. If not a member of the group, add the process to the process list of the group
	 *	8. Update other attributes of the group - Number of process that belong to the group
	 *	9. If a new group is created - Update total number of groups created	*/
		
	
	//endpoint_t pid = mproc[who_p].mp_pid;
	
	int openGroupId = m_in.m1_i1;
	multicast_group *group = NULL;
	int processAlreadyInGroup = 0;
	
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(openGroupId == multiCastGroup[i].multiCastGroupID)
		{			
			group = &multiCastGroup[i];
			break;
		}
	}
	
	if(group == NULL)
	{
		group = &multiCastGroup[currentMultiCastGroupCount];
		
		group->multiCastGroupID = openGroupId;
		group->currentMemberCount = 0;
		group->groupMembers[group->currentMemberCount].epId = who_e;
		group->groupMembers[group->currentMemberCount].prevRead = NULL;
		group->msgQueue.front = NULL;
		group->msgQueue.rear = NULL;
		group->msgQueue.qsize = 0;
		group->currentMemberCount++;
		
		currentMultiCastGroupCount++;
		
		return OK;
	}
	
	for(int i = 0; i < group->currentMemberCount; i++)
	{
		if(who_e == group->groupMembers[i].epId)
		{
			processAlreadyInGroup = 1;
			break;
		}
	}
	if(!processAlreadyInGroup)
	{
		group->groupMembers[group->currentMemberCount].epId = who_e;
		group->groupMembers[group->currentMemberCount].prevRead = NULL;
		group->currentMemberCount++;
	}

	return OK;
}

/*===========================================================================*
 *				do_closegroup			     *
 *===========================================================================*/
int
do_closegroup(void)
{
	/* 	1. Get the pid of the caller receiver process
	 *	2. Read the input group id passed by the receiver process - m_in.m1_i1
	 *	3. Check if the group with the input id is present
	 *	4. If group is not present, return an error message - "Group with the specified id does not exist"
	 *	5. If group is present, check if the current process is a member of the process,
	 *		6. If the process is a member of the group, remove the process from the process list of the group
	 *		7. If not a member of the group, return an error message - "Process does not belong to the group with the specified id"
	 *	8. Update other attributes of the group - Number of process that belong to the group
	 *	9. If the process was the only member of the group, delete the group - Update groups, and total number of groups	*/
	 
	//endpoint_t pid = mproc[who_p].mp_pid;
	
	int closeGroupId = m_in.m1_i1;
	multicast_group *group = NULL;
	int processPresentInGroup = 0;
	int processIndexInGroup = 0;
	int groupIndex = 0;
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(closeGroupId == multiCastGroup[i].multiCastGroupID)
		{
			group = &multiCastGroup[i];
			groupIndex = i;
			break;
		}
	}
	
	if(group == NULL)
	{
		return EINVAL;
	}
	 
	for(int i = 0; i < group->currentMemberCount; i++)
	{
		if(who_e == group->groupMembers[i].epId)
		{
			processPresentInGroup = 1;	
			processIndexInGroup = i;			
			break;
		}
	}
	
	if(!processPresentInGroup)
	{
		return OK;
	}
	
	for(int i = processIndexInGroup; i < group->currentMemberCount-1; i++)
	{
		group->groupMembers[i] = group->groupMembers[i+1];
	}
	
	group->currentMemberCount--;
	
	
	if(group->currentMemberCount == 0)
	{
		for(int i = groupIndex; i < currentMultiCastGroupCount-1; i++)
		{
			multiCastGroup[i] = multiCastGroup[i+1];	
			//multiCastGroup[i].multiCastGroupID = multiCastGroup[i+1].multiCastGroupID;
		}
		
		currentMultiCastGroupCount--;
	}

	return OK;
}

/*===========================================================================*
 *				do_recovergroup			     *
 *===========================================================================*/
int
do_recovergroup(void)
{
	mipc_queue* target_queue = NULL;
	multicast_group *group = NULL;
	unsigned short gid = m_in.m1_i1;
	int r = OK;
	mipc_queueentry* msg = NULL;
	int groupIndex = 0;
	
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(gid == multiCastGroup[i].multiCastGroupID)
		{			
			group = &multiCastGroup[i];
			groupIndex = i;
			break;
		}
	}
	
	if(group != NULL)
	{
		target_queue = &(group->msgQueue);
		
		for(int i = 0; i < group->currentMemberCount; i++)
		{
			//Check for the unresponsive receivers and remove them
			if(group->groupMembers[i].prevRead == NULL)
			{
				for(int j = i; j < group->currentMemberCount-1; j++)
				{
					group->groupMembers[j] = group->groupMembers[j+1];
				}
				
				group->currentMemberCount--;
				
			}
		}
		
		if(target_queue->front->readCount >= group->currentMemberCount)
		{
			for(int i = 0; i < group->currentMemberCount; i++)
			{
				if(target_queue->front == group->groupMembers[i].prevRead)
				{
					group->groupMembers[i].prevRead = NULL;
				}
			}
			Dequeue(target_queue);	
		}
		
		if(group->currentMemberCount == 0)
		{
			for(int j = groupIndex; j < currentMultiCastGroupCount-1; j++)
			{
				multiCastGroup[j] = multiCastGroup[j+1];
			}
			currentMultiCastGroupCount--;
		}
	}
	else
	{
		r = EINVAL;
	}
	
	return r;
}

/*===========================================================================*
 *				do_getgroupinfo			     *
 *===========================================================================*/
int
do_getgroupinfo(void)
{	
	multicast_group *group = NULL;
	unsigned short gid = m_in.m1_i1;
	int r = OK;
	
	
	for(int i = 0; i < currentMultiCastGroupCount; i++)
	{
		if(gid == multiCastGroup[i].multiCastGroupID)
		{			
			group = &multiCastGroup[i];
			break;
		}
	}
	
	if(group != NULL)
	{
		int* buf = (int*)m_in.m1_p1;
		
		r = sys_datacopy(SELF, (vir_bytes)&(group->currentMemberCount), who_e, (vir_bytes)buf, sizeof(int));
			
		if (r == OK)
		{
			buf = (int*)m_in.m1_p2;
			
			if(buf != NULL)
			{
				for(int j = 0; j < group->currentMemberCount; j++)
				{
					//printf("\nbuf[%d] : %p\n", j, buf);
					r = sys_datacopy(SELF, (vir_bytes)&(group->groupMembers[j].epId), who_e, (vir_bytes)buf, sizeof(int));
					buf++;
				}
			}
		}	
	}
	else
	{
		r = EINVAL;
	}
	
	return OK;
} 
