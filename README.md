# Readers-Writers-Lock-with-Priority-Inheritance
CSC 501 Operating Systems
North Carolina State University

Develop locks and perform mutex lock synchronization between multiple threads.
Readers/writer locks are used to synchronize access to a shared data structure. A lock can be acquired for read or write operations.
A lock acquired for reading can be shared by other readers, but a lock acquired for writing must be exclusive. You have been provided with the standard semaphore implementation for XINU. Task is to extend the XINU semaphore to implement the readers/writer lock semantics. Although, the standard semaphores implemented in XINU are quite useful, there are some issues with the XINU semaphores which we will try to fix in this assignment. XINU semaphores do not distinguish between read accesses, which can co-exist, and write accesses, which must be exclusive. Another problem with XINU's semaphores occurs when a semaphore is deleted at a time when it has processes waiting in its queue. In such situation, sdelete awakens all the waiting processes by moving them from the semaphore queue to the ready list. As a result, a process that is waiting for some event to occur will be awakened, even though the event has not yet occurred.


