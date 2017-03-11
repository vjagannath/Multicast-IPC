#ifndef MIPC_H
#define MIPC_H

//Includes
#include <lib.h>

//Macro Definitions
#define MAX_MIPC_QUEUE_LENGTH 256

//Type Definitions
typedef struct _mipc_queueentry 
{
	message data;
	int readCount;
	struct _mipc_queueentry* next;
}mipc_queueentry;

typedef struct _mipc_queue 
{
	// Two variables to store address of front and rear nodes. 
	mipc_queueentry* front;
	mipc_queueentry* rear;
	unsigned int qsize;
}mipc_queue;

typedef struct _multicast_group_member
{
	endpoint_t epId;
	mipc_queueentry* prevRead;
}multicast_group_member;

/* Member variables for multicast group information*/
typedef struct _multicast_group
{
	int multiCastGroupID; /* group id */
	multicast_group_member groupMembers[NR_PROCS]; /* processes that belong to the group */
	int currentMemberCount; /* count of number of processes in the group */
	mipc_queue msgQueue; /* message queue for the group */
}multicast_group;

//Global Variables
multicast_group multiCastGroup[NR_PROCS];
int currentMultiCastGroupCount = 0;


#endif //MIPC_H