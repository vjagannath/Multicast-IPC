#include "multicast.h"

int msend(unsigned short gid, message* msg_ptr)
{
	message m;
	
	m.m1_i1 = gid;
	m.m1_p1 = (char *)msg_ptr;
	
	return(_syscall(PM_PROC_NR, PM_MSEND, &m));
}

int mreceive(unsigned short gid, message* msg_ptr)
{
	message m;
	
	m.m1_i1 = gid;
	m.m1_p1 = (char *)msg_ptr;
	
	return(_syscall(PM_PROC_NR, PM_MRECEIVE, &m));
}
int opengroup(unsigned short gid)
{
	message m;
	
	m.m1_i1 = gid;
	
	// Invoke the open group system call by passing group id information to the server process
	return(_syscall(PM_PROC_NR, PM_OPENGROUP, &m));
}

int closegroup(unsigned short gid)
{
	message m;
	
	m.m1_i1 = gid;

	// Invoke the close group system call by passing group id information to the server process
	return(_syscall(PM_PROC_NR, PM_CLOSEGROUP, &m));
}
int recovergroup(unsigned short gid)
{
	message m;

	m.m1_i1 = gid;
	
	return(_syscall(PM_PROC_NR, PM_RECOVERGROUP, &m));
}

int getgroupInfo(unsigned short gid, int* gSize, int* gMembers)
{
	message m;
	int status = 0;
	int buff_size = *gSize;
	
	m.m1_i1 = gid;
	m.m1_p1 = (char*)gSize;
	m.m1_p2 = NULL;
	
	status = _syscall(PM_PROC_NR, PM_GETGROUPINFO, &m);
	
	if(status == 0)
	{
		if(buff_size < *gSize)
		{
			status = ENOBUFS;
		}
		else
		{
			m.m1_i1 = gid;
			m.m1_p1 = (char*)gSize;
			m.m1_p2 = (char*)gMembers;
			
			status = _syscall(PM_PROC_NR, PM_GETGROUPINFO, &m);
		}
	}
	
	return status;
	
}
