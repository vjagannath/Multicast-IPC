# Multicast-IPC
Multicast Inter Process Communication framework in Minix Operating System for sending messages between group of user processes

1. Scope
The scope of the project includes developing a set of Inter Process Communication system calls that allows an application process to send messages to a group of processes. This overcomes the current kernel supported IPC restrictions pertaining to a single receiver.

2. Operations
The project aims at providing a support for multicast IPC’s. This is achieved through a set of new system calls. At the heart of this set of IPC’s is a pair of msend and mreceive system calls. A user process calls msend to send a message to a group of other user processes who call mreceive to retrieve the message. The user process calls opengroup to create a new group or get added to an existing group. A process can belong to more than one group. The closegroup is called from a process to exit from a group.

The new system calls implemented in this perspective include:

 msend: non-blocking system call to send a message to a group of processes.

 mreceive: blocking system call to receive a message from a group.

 opengroup: system call made by the process to create a new group or add itself to an existing group

 closegroup: system call made by the process to exit from a group if it belongs to a group

 recovergroup: system call to recover from a deadlock-where the sender is trying to send messages to a group but the message buffer of the group is full. No receiver process is executing receive. The sender will be blocked from sending.

 getgroupinfo: system call to retrieve available group information.

3. Design Considerations
msend, mreceive

The multicast group IPC implementation provisions receiver consistent message passing between sender and a group of receivers i.e; all the receivers receive the messages in the same order. No two processes in the same group receive the messages from a common sender in a different order. The send system call is non-blocking while the receive system is blocking to enable a concurrent asynchronous data transfer without delay. This also avoids system buffering and allows computations and communication to overlap, which generally leads to improved performance.

opengroup

System call to support creation of a multicast group where in, a process requests to join a group by providing a group id. If there exists a group with the input group id, the process gets added to the group, else a new group with the process is created.

closegroup

System call that provisions a process to exit from group where in, a process is prompted for the group id of the group which it wants to exit from. If the group exists and the process belongs to the group, it is removed from the group. And after the process is removed from the process, if the group contains no more processes, the group is deleted.

recovergroup

System call to recover from a deadlock that arises when a receiver in a group is not receiving thus blocking the sender from sending more messages to the group and blocking other receiver processes from receiving other messages in the queue. This is achieved by removing the blocking process from the group and continuing the operations with other processes present in the group.

getgroupinfo

System call to retrieve group information of the input group id - processes which belong to the group, and number of processes in the group
