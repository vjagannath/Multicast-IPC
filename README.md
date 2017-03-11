# Multicast-IPC
Multicast Inter Process Communication framework in Minix Operating System for sending messages between group of user processes

Scope
The scope of the project includes developing a set of Inter Process Communication system calls that allows an application process to send messages to a group of processes. This overcomes the current kernel supported IPC restrictions pertaining to a single receiver.

Operations
The project aims at providing a support for multicast IPC’s. This is achieved through a set of new system calls. At the heart of this set of IPC’s is a pair of msend and mreceive system calls. A user process calls msend to send a message to a group of other user processes who call mreceive to retrieve the message. The user process calls opengroup to create a new group or get added to an existing group. A process can belong to more than one group. The closegroup is called from a process to exit from a group.
The new system calls implemented in this perspective include:
 msend: non-blocking system call to send a message to a group of processes.
 mreceive: blocking system call to receive a message from a group.
 opengroup: system call made by the process to create a new group or add itself to an existing group
 closegroup: system call made by the process to exit from a group if it belongs to a group
 recovergroup: system call to recover from a deadlock-where the sender is trying to send messages to a group but the message buffer of the group is full. No receiver process is executing receive. The sender will be blocked from sending.
 getgroupinfo: system call to retrieve available group information.
Design Considerations
msend, mreceive
The multicast group IPC implementation provisions receiver consistent message passing between sender and a group of receivers i.e; all the receivers receive the messages in the same order. No two processes in the same group receive the messages from a common sender in a different order. The send system call is non-blocking while the receive system is blocking to enable a concurrent asynchronous data transfer without delay. This also avoids system buffering and allows computations and communication to overlap, which generally leads to improved performance.
Pseudocode: msend
• Check if the group with the input group id exists - m_in.m1_i1
• If the group exists, get the destination queue of the group.
• Copy the incoming message from the sender process to the rare of the destination queue of the group – m_in.m1_p1.
• Else, return.
Pseudocode: mreceive
• Check if the group with the input group id exists - m_in.m1_i1
• If the group exists, check if the caller receiver process is a member of the group.
• If present, read the next to be read message by the process. If no message exists, then wait until message appears at the queue.
• Check if all the members have read the message.
• If yes, the dequeue the message from the group’s destination buffer.
opengroup
System call to support creation of a multicast group where in, a process requests to join a group by providing a group id. If there exists a group with the input group id, the process gets added to the group, else a new group with the process is created.
Pseudocode:
• Get the process id of the caller receiver process
• Read the input group id passed by the receiver process - m_in.m1_i1
• Check if the group with the input id is already present
• If group is not present, create a new group with the group id and add the process to the process list of the group
• If group is present, check if the current process is already a member of the process,
• If already a member of the group, return with a message "Process is already a member of this group"
• If not a member of the group, add the process to the process list of the group
• Update other attributes of the group - Number of process that belong to the group
• If a new group is created - Update total number of groups created
closegroup
System call that provisions a process to exit from group where in, a process is prompted for the group id of the group which it wants to exit from. If the group exists and the process belongs to the group, it is removed from the group. And after the process is removed from the process, if the group contains no more processes, the group is deleted.
Pseudocode:
• Get the process id of the caller receiver process
• Read the input group id passed by the receiver process - m_in.m1_i1
• Check if the group with the input id is present
• If group is not present, return an error message - "Group with the specified id does not exist"
• If group is present, check if the current process is a member of the process,
• If the process is a member of the group, remove the process from the process list of the group
• If not a member of the group, return an error message - "Process does not belong to the group with the specified id"
• Update other attributes of the group - Number of process that belong to the group
• If the process was the only member of the group, delete the group - Update groups, and total number of groups
recovergroup
System call to recover from a deadlock that arises when a receiver in a group is not receiving thus blocking the sender from sending more messages to the group and blocking other receiver processes from receiving other messages in the queue. This is achieved by removing the blocking process from the group and continuing the operations with other processes present in the group.
Pseudocode:
• Get the group id of the group to which user is blocked from sending messages to - m_in.m1_i1
• Get the group structure corresponding to the group id
• Identify the blocking receiver process and remove it from the group
• Update other attributes of the group - Number of process that belong to the group along with msg group list for message tracking
• If after the blocking process is removed, if the group contains no element, delete the group
getgroupinfo
System call to retrieve group information of the input group id - processes which belong to the group, and number of processes in the group
Pseudocode:
• Iterate over all the available groups and print the group details of the group corresponding to the input group id.
• The second parameter and third parameter represents the group size and group members corresponding to the group for which the group info is requested for by the user process.
