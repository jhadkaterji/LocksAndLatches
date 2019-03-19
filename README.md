# LocksAndLatches
This program implements a reentrant lock, a countdown latch, and a reentrant readers-writers lock, 


Reentrant Lock - 
A reentrant lock is a mechanism for providing exclusive access to a resource shared by a set of threads. The mechanism is similar to a pthread mutex, but it is reentrant, meaning that a thread that holds the lock can re-lock the lock. The lock is not made available to another thread until the holder of the lock has unlocked the lock the same number of times as it locked it.


Countdown Latch. - 
A countdown latch is a mechanism for coordinating the exeuction of a set of threads. The latch is created with a non-negative count. The latch remains closed until its count is driven down to zero. Threads that wait on the latch will be blocked until the latch opens when its count reaches zero.



Reentrant Readers-Writers Lock - 
A reentrant readers-writers lock is a mechanism for controlling access to a resource shared by a set of threads. The mechanism provides two locks, one for threads that want to read the resource and one for threads that want to write the resource. Multiple readers can hold the readers lock, but only if no thread holds the writers lock. Only one thread can hold the writers lock.

Priority is given to writers, meaning that when the writers lock is unlocked, if there are waiting writers, one of them is given the writers lock, even if there have been readers waiting longer. Writers must wait for all readers to release the lock, but once there is a writer waiting no additional readers will be given the readers lock. However, if a thread already holds the readers lock, wants to re-lock it, and there is a writer waiting, the thread holding the readers lock will be allowed to re-lock it.



