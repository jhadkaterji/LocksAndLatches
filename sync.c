//Jhad Katerji
//Program 5
//Professor Hatcher
//CS 520
//Due November 4, 2018

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "sync.h"


//////////////////////////////////////////
//////////////////////////////////////////
//////////////LAB 9///////////////////////
//////////////////////////////////////////
//////////////////////////////////////////

//Re-entrant Lock Implimentation - Can be re-locked. 
//Not made available to another thread until holder of lock has unlocked
// lock same number of times as locked. 

//Create the struct for the reeentrant lock. 
struct reentLock
{
   pthread_mutex_t mu; //Protect shared struct
   pthread_cond_t cv; //Condition variable
   pthread_t self; //Call to self to find out who you are 
   int count; //Number of times the lock has been locked. 
};


//Create lock function. 
void *createLock(void)
{
    struct reentLock *r = malloc (sizeof(struct reentLock));//Malloc the struct
    pthread_mutex_init (&r->mu, NULL);//Initialize the mutex
    pthread_cond_init(&r->cv, NULL);//Initialize the condition variable
    r -> count = 0;//Set the count = to 0. 
    return r; // Return the created lock. 
}


//Lock lock function. 
void lockLock(void *lock)
{
	struct reentLock *r = lock;// The struct we will be using for the function.
    pthread_mutex_lock ( &r-> mu );// Calling the mutex Lock to start the fucntion. 

    if (r -> count > 0)// This is checking to see if the lock is even locked. 
		{
    		if (pthread_equal (r -> self, pthread_self() ) != 0)// Checking if I am the owner
    			{
        			r -> count += 1; // Increment the lock count by 1. 
 				}
			else
 				{
		 			while ( r-> count > 0) //While the lock is locked...
						{
							pthread_cond_wait (&r -> cv, &r -> mu);
						}
					r -> self = pthread_self();// Record my thread ID as the owner. 
					r -> count = 1;//Increment lock counter by 1.
				}
		}  
	else
		{
			r -> self = pthread_self();// Record my thread ID as the owner. 
			r -> count = 1;//Increment lock counter by 1.
		}
	
	pthread_mutex_unlock ( &r-> mu );// Mutex unlock to end the fucntion. 
}


//Creating the unlockLock function. 
int unlockLock(void *lock)
{
	int returnVar = 0; //This is a variable that I will use for returning things later on. 
	struct reentLock *r = lock;//The struct that will be refrenced in this function. 

	if(r->count == 0)// If the lock has not yet been locked
		{
			return 0;// Return 0 because you can't unlock an unlcoked lock.
		}

	pthread_mutex_lock ( &r-> mu );// Begin the function with a mutex lock. 
	if (r -> count > 0)
		{
			if ( pthread_equal (r -> self, pthread_self() ) != 0) //Checking if I am the owner
				{
					//returns a non-zero value if they are equal and zero otherwise.
					r -> count --;// Decrement the counter by 1.
					returnVar = 1;// Set the return variable equal to 1 because it was sucessful. 
				}

			if(r-> count ==0) //Checkfing to see if the count is equal to 0 after the count has been decremented. 
				{
					pthread_cond_signal (&r -> cv);// DO a signal function. 
				}
		} 
	pthread_mutex_unlock ( &r-> mu );//Mutex unlock to end the function. 
	return returnVar; //Return my return variable. 	
}

//Create the freeLock function
void freeLock(void *lock)
{
	struct reentLock *r = lock;//The lock that will be used in this function. 
	free (r);//Free from memory. 
	//exit (-1); //If a bad handle is passed to this routine, the behavior is undefined.
}


/////COUNTDOWN LATCH//////
//Create the struct for the latch. 
struct cdLatch
{
   pthread_mutex_t mu; //Protect shared struct
   pthread_cond_t cv; //Condition variable
   int count; //Number of times the lock has been locked. 
};

// create a countdown latch, returning a handle for manipulating it
void *createCountdownLatch(unsigned int count)
{
	struct cdLatch *l = malloc (sizeof(struct cdLatch));//Malloc the struct
    pthread_mutex_init (&l->mu, NULL);//Initialize the mutex
    pthread_cond_init(&l->cv, NULL);//Initialize the condition variable
    l -> count = count;//Set the count = to 0. 
    return l; // Return the created lock. 
}

// countdown a latch
void countdown(void *latch)
{
	struct cdLatch *l = latch;//The struct that will be refrenced in this function. 
	pthread_mutex_lock ( &l-> mu );// Calling the mutex Lock to start the fucntion. 

	if (l -> count > 0) //If the count is greater than 0, decrement the count. 
		{
			l -> count --;
		}

	if (l -> count == 0)
		{
			pthread_cond_broadcast (&l -> cv);
		}
	
	pthread_mutex_unlock ( &l-> mu );// Mutex unlock; this no longer needs to be protected. 
}

// await the opening of the latch
void await(void *latch)
{
	struct cdLatch *l = latch;//The struct that will be refrenced in this function. 
	pthread_mutex_lock ( &l-> mu );// Calling the mutex Lock to start the fucntion.

	while (l ->count > 0)
	{
		pthread_cond_wait (&l -> cv, &l -> mu);
	}
	pthread_mutex_unlock ( &l-> mu );
}

// free the memory for a latch
void freeCountdownLatch(void *latch)
{
	struct cdLatch *l = latch;//The latch that will be used in this function. 
	free (l);//Free from memory. 
	//exit (-1); //If a bad handle is passed to this routine, the behavior is undefined.
}

//////////////////////////////////////////////
//////////////////////////////////////////////
///////////PROGRAM 5//////////////////////////w
//////////////////////////////////////////////
//////////////////////////////////////////////

//Create the struct for the RWLock. 
struct rwLock
{
   pthread_mutex_t mu; //Protect shared struct
   pthread_cond_t cvW; //Condition variable1 - writing
   pthread_cond_t cvR; //Condition variable2 - reading
   pthread_t writeLockOwner; //Who owns the write lock.  
   int writeLockCount; //Write lock count. 
   int readLockCount; // Read lock count.
   int waitingWcount;// Waiting writer count.  
   pthread_key_t key; //Key used for thread specific data. //Reader count for each thread.
};

// create a reader-writer lock, returning a handle for manipulating it
void *createRWLock(void)
{
	struct rwLock *rwl = malloc (sizeof(struct rwLock));//Malloc the struct. 
	pthread_mutex_init (&rwl->mu, NULL);//Initialize the mutex. 
    pthread_cond_init(&rwl->cvW, NULL);//Initialize the first condition variable.
	pthread_cond_init(&rwl->cvR, NULL);//Initialize the second condition variable.  
	rwl -> writeLockCount = 0;// Set write lock count to 0. 
	rwl -> readLockCount = 0;// Set read lock count to 0. 
	rwl -> waitingWcount = 0;// Set the waiting writer count to 0.
	rwl -> writeLockOwner = 0;// Initialize the write lock owner to get rid of errors.  
	pthread_key_create (&rwl -> key, NULL); // initialize the key. 
	return rwl;// Return the newly created struct. 
}

//Function to lock the RW LOCK. 
void lockRLock(void *lock)
{
	struct rwLock *rwl = lock;
	pthread_mutex_lock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.

	while (rwl -> writeLockCount >0 || rwl -> waitingWcount > 0)//Any writers waiting?
		{
			pthread_cond_wait (&rwl -> cvR, &rwl -> mu);//Call a cond wait on Reader's var. 
		}
	unsigned long getSpecVal;// Long variable to store the get specific(). 
	getSpecVal = (long) pthread_getspecific (rwl -> key);//Val to check if I'm on the list. 

	if (getSpecVal == 0)//Checking if I'm not on the list, add me to the list in my struct. 
		{
			rwl -> readLockCount += 1;// Incrememnt the read lock count. 
		}
	
	getSpecVal += 1;
	if ( (pthread_setspecific(rwl -> key, (void *)(getSpecVal)) ) != 0 )
		{
			perror ("Error, Set Specific Failed!");
			exit (-1);
		}
	pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Unlock to end the fucntion.
}


//Function to lock the Writer Lock. 
// lock the writer lock of a RWLock
// block if the lock is not available
void lockWLock(void *lock)
{
	struct rwLock *rwl = lock;
	pthread_mutex_lock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.
	
	
	if ( pthread_equal (rwl -> writeLockOwner, pthread_self() ) != 0) //Checking if I am the owner
		{
			//If they are equal, a non zero value is returned. 
			//Just increment the count. 
			rwl -> writeLockCount += 1; //Write lock count.
		}	
	
	else
		{
			rwl -> waitingWcount += 1;// Increment the waiting writer count
			while (rwl -> writeLockCount >0 || rwl -> readLockCount > 0)//Any writers waiting?
				{
					if(pthread_equal (rwl -> writeLockOwner, pthread_self() ) == 0)
					{
						pthread_cond_wait (&rwl -> cvW, &rwl -> mu);// Calling condition wait.
					}
				}
			rwl -> waitingWcount -= 1;// Decrement the waiting writer count
			rwl -> writeLockOwner = pthread_self();// Record my thread ID as the owner. 
			rwl -> writeLockCount += 1; //Write lock count.
			
		}
		pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Unlock to end the fucntion.
}


//Function to unlock the Reader lock. 
// perform one unlock of the reader lock of a RWLock
int unlockRLock(void *lock)
{
	struct rwLock *rwl = lock;
	pthread_mutex_lock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.

	if (rwl -> readLockCount == 0)
		{
			pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.
			return 0;
		}

	
	unsigned long getSpecVal;
	getSpecVal = (long) pthread_getspecific (rwl -> key);

	if (getSpecVal == 0)//Checking if I'm not on the list.
		{
			pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.
			return 0;// If I'm not on the list, return 0. 
		}
		
	if ( (pthread_setspecific(rwl -> key, (void *)(getSpecVal-1)) ) != 0 )
		{
			perror ("Error, Set Specific Failed!");
			exit (-1);
		}
		getSpecVal -= 1;
		if (getSpecVal == 0)
		{
			rwl -> readLockCount --;
			
			if (rwl ->readLockCount == 0)
			{
				pthread_cond_signal (&rwl -> cvR);// Do a signal function. 
			}
			
		}
		

	pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Unlock to end the fucntion.
	return 1;
}


//Function to unlock the Writer lock. 
// perform one unlock of the writer lock of a RWLock
int unlockWLock(void *lock)
{
	struct rwLock *rwl = lock;
	pthread_mutex_lock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.

	if (rwl -> writeLockCount == 0)
		{
			pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion.
			return 0;
		}

if ( pthread_equal (rwl -> writeLockOwner, pthread_self() ) == 0) //Checking if I am the owner
		{
			
			//If they are equal, a non zero value is returned. 
			//rwl -> writeLockCount += 1;//Just increment the count.
			pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Lock to start the fucntion. 
			return 0; //Write lock count.
		}	
		
		rwl -> writeLockCount --;
		
		if (rwl -> writeLockCount == 0)
			{
				rwl -> writeLockOwner = 0;
				if (rwl -> waitingWcount > 0)
					{
						pthread_cond_signal (&rwl -> cvW);// Do a signal function.
					} 
				
				else
					{
						pthread_cond_broadcast (&rwl -> cvR);
					}
			}
			
	pthread_mutex_unlock ( &rwl-> mu );// Calling the mutex Unlock to end the fucntion.
	return 1;
}


// free the memory for a RWLock
void freeRWLock(void *lock)
{
	struct rwLock *rwl = lock;
	free (rwl);//Free the memory for the lock that was malloced. 
}
