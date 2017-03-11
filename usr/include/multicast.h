#ifndef MULTICAST_H
#define MULTICAST_H
#include <lib.h>

int msend(unsigned short gid, message* msg_ptr);
int mreceive(unsigned short gid, message* msg_ptr);
int opengroup(unsigned short gid);
int closegroup(unsigned short gid);
int recovergroup(unsigned short gid);
int getgroupInfo(unsigned short gid, int* gSize, int* gMembers);

#endif //MULTICAST_H