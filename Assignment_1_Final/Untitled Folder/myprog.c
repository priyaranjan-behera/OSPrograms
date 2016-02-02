#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<string.h>
#define MEM 8000
#define ARR_SIZE 10

int threadCount = 0;
int semCount = 0;
int start;

typedef struct MyThread {
    ucontext_t context;
    int id;
    struct MyThread * next;
    int parent;
    int blockedFor[ARR_SIZE];
    int semaphoreBlocked;
} MyThread;

typedef struct MySemaphore {
	int semaphoreId;
	int semaphoreValue;
	struct MySemaphore * next;
} MySemaphore;

MyThread* ready_queue;
MyThread* blocked_queue;
MyThread* semaphore_blocked_queue;
MySemaphore* semaphore_queue;

ucontext_t controller;

void printQueues()
{
	MyThread* trial;
	int cnt;
//
		trial = ready_queue;
		cnt = 0;
		printf("\n*********************************");
		printf("\nPrinting Ready Queue at Start: ");
		while(trial != NULL)
		{	
			printf(" %d ", trial->id);
			trial = trial->next;
			cnt++;
		}


		printf("\nThe Total Count of Thread in Ready_Queue is %d", cnt);
		printf("\n*********************************");

		trial = blocked_queue;
		cnt = 0;
		printf("\n*********************************");
		printf("\nPrinting Block Queue at Start: ");
		while(trial != NULL)
		{	
			//printf(" %d ", trial->id);
			trial = trial->next;
			cnt++;
		}


		printf("\nThe Total Count of Thread in Block_Queue is %d", cnt);
		printf("\n*********************************");
		

		trial = semaphore_blocked_queue;
		cnt = 0;
		printf("\n*********************************");
		printf("\nPrinting Semaphore Block Queue at Start: ");
		while(trial != NULL)
		{	
			printf(" (%d - Blocked for Sem Id: %d) ", trial->id, trial->semaphoreBlocked);
			trial = trial->next;
			cnt++;
		}


		printf("\nThe Total Count of Thread in Semaphore Block_Queue is %d", cnt);
		printf("\n*********************************");

		MySemaphore* sem_trial = semaphore_queue;
		cnt = 0;
		printf("\n*********************************");
		printf("\nPrinting Semaphore BQueue at Start: ");
		while(sem_trial != NULL)
		{	
			printf(" (%d - Semaphore Id- Sem Value: %d) ", sem_trial->semaphoreId, sem_trial->semaphoreValue);
			sem_trial = sem_trial->next;
			cnt++;
		}


		printf("\nThe Total Count of Thread in Semaphore Queue is %d", cnt);
		printf("\n*********************************");
}



MyThread* insertNode(MyThread* head)
{

	MyThread* trav;

	trav = head;

	if(head == NULL)
	{
		head = malloc(sizeof(MyThread));
		return head;
	}
	else
	{
		while(trav->next != NULL)
			trav = trav-> next;

		trav->next = malloc(sizeof(MyThread));
		return trav->next;
	}

}

void insertIntoBlockedQueue(MyThread* newNode)
{

	////printf("\n Inserting to Block Queue. Thread: %d", newNode->id);
	////printf("\n This Thread is waiting on: ");
	int i;
	for (i=0; i<ARR_SIZE; i++)
	{
		////printf(" %d ", newNode->blockedFor[i]);
	}	
	if(blocked_queue == NULL)
	{
		blocked_queue = ready_queue;
		return;
	}
	
	MyThread* trav;
	trav = blocked_queue;
	while(trav->next != NULL)
		trav = trav->next;
	trav->next = newNode;
	trav->next->next = NULL;	
}


void insertIntoSemaphoreBlockedQueue(MyThread* newNode)
{

	////printf("\n Inserting to Block Queue. Thread: %d", newNode->id);
	////printf("\n This Thread is waiting on: ");
	
	if(semaphore_blocked_queue == NULL)
	{
		semaphore_blocked_queue = ready_queue;
		return;
	}
	
	MyThread* trav;
	trav = semaphore_blocked_queue;
	while(trav->next != NULL)
		trav = trav->next;
	trav->next = newNode;
	trav->next->next = NULL;	
}

void insertIntoSemaphoreQueue(MySemaphore* newNode)
{

	////printf("\n Inserting to Block Queue. Thread: %d", newNode->id);
	////printf("\n This Thread is waiting on: ");
	
	if(semaphore_queue == NULL)
	{
		semaphore_queue = newNode;
		semaphore_queue->next = NULL;
		return;
	}
	
	MySemaphore* trav;
	trav = semaphore_queue;
	while(trav->next != NULL)
		trav = trav->next;
	trav->next = newNode;
	trav->next->next = NULL;	
}

MySemaphore* getSemaphoreFromId(int id)
{
	MySemaphore* trav = semaphore_queue;
	while(trav != NULL)
	{
		if(trav->semaphoreId == id)
			return trav;
		trav = trav->next;
	}

	return NULL;
}

int deleteSemaphore(int id)
{
	//printf("Inside Delete Semaphore");
	if(semaphore_queue == NULL)
		return 0;

	if(semaphore_queue->semaphoreId == id)
	{
		semaphore_queue = semaphore_queue->next;
		return 1;
	} 

	MySemaphore* trav = semaphore_queue;
	while(trav->next != NULL)
	{
		if(trav->next->semaphoreId == id)
		{
			trav->next = trav->next->next;
			return 1;
		}
		trav = trav->next;
	}
	return 0;
}


void insertIntoReadyQueue(MyThread* newNode)
{

	////printf("\n Inserting to Ready Queue. Thread: %d", newNode->id);
	////printf("\nChecking if ready_queue is NULL");

	if(ready_queue == NULL)
	{
		ready_queue = newNode;
		newNode->next = NULL;
		////printf("Ready Queue was null");
		return;
	}
	
	MyThread* trav;
	trav = ready_queue;
	while(trav->next != NULL) // make changes so as to create a 
		trav = trav->next;
	trav->next = newNode;
	trav->next->next = NULL;
	
}

int presentInReadyQueue(int id)
{
	MyThread* trav;
	trav = ready_queue;
	//printf("CPPRQ");
	while(trav!=NULL)
	{
		if(trav->id == id)
			return 1;
		trav = trav->next;
	}

	return 0;
}

int MyThreadJoin(MyThread thread)
{
	if (thread.parent != ready_queue->id)
		return -1; 
	if (!presentInReadyQueue(thread.id)) 
		return 0;

	memset(&(ready_queue->blockedFor[0]), 0, sizeof((ready_queue->blockedFor)));
	ready_queue->blockedFor[0] = thread.id;
	MyThread* trav = ready_queue->next;
	MyThread* temp = ready_queue;
	insertIntoBlockedQueue(ready_queue);
	ready_queue->next = NULL;
	ready_queue = trav;
	////printf("\n Printing the Queues from Thread Join: ");
	//printQueues();
	swapcontext(&temp->context, &ready_queue->context);

	return 0;
}

void MyThreadJoinAll()
{
	//printQueues();
	MyThread* trav = ready_queue;
	int children[ARR_SIZE];
	
	int i, counter = 0;
	while(trav != NULL)
	{
		if((trav->parent == ready_queue->id) && !(ready_queue->id == 0 && trav->id == 0)) //second term to handle root node joining
		{
			children[counter++] = trav->id;
		}
		trav = trav->next;
	}
	
	////printf("\nThe counter returns %d", counter);
	//printQueues();

	if(counter == 0)
		return;
	memset(&(ready_queue->blockedFor[0]), 0, sizeof((ready_queue->blockedFor)));
	for(i=0; i<counter; i++)
	{	
		ready_queue->blockedFor[i] = children[i];
	}

	trav = ready_queue->next;
	MyThread* temp = ready_queue;
	insertIntoBlockedQueue(ready_queue);
	ready_queue->next = NULL;
	ready_queue = trav;
	////printf("\n Printing the Queues from Thread Join: ");
	//printQueues();
	swapcontext(&temp->context, &ready_queue->context);


}



int presentInSemaphoreBlockedQueue(int id)
{
	MyThread* trav;
	trav = semaphore_blocked_queue;
	//printf("CPPSBQ");
	while(trav!=NULL)
	{
		if(trav->id == id)
		{
			//printf("CPPSBQ-Returing 1");
			return 1;
		}
		trav = trav->next;
	}
	return 0;
}

MyThread* moveNextNode(MyThread *head)
{
	head = head->next;
}

MyThread MyThreadCreate (void(*start_funct)(void *), void *args) //Finalizes Create Thread Function
{
	printf("Entering Thread Create");
	MyThread* child = insertNode(ready_queue);
	getcontext(&(child->context));
	child->context.uc_stack.ss_sp=malloc(MEM);
 	child->context.uc_stack.ss_size=MEM;
 	child->context.uc_stack.ss_flags=0;
	child->id = threadCount++;
	child->parent = ready_queue->id;
	child->context.uc_link=&controller;
	printf("CP1");
	printQueues();
	makecontext(&(child->context), (void*)start_funct, 1, args);
	printf("CP2");	
	printQueues();
	printf("\n The size of the mythread object is: %d",sizeof(*child));
	printf("\n The size of the int is: %d",sizeof(int));

	MyThread ret;
	ret.id = child->id;
	ret.parent = child->parent;

	printf("\n The size of the ret is: %d",sizeof(ret));
	printf("After Allocation");
	printQueues();

	printf("\nHi, this is the %d thread getting created, my parent is: %d", child->id, ready_queue->id);
	printf("Before Returning");
	printQueues();
	return;
}

void MyThreadYield(void) // Finalized Yield Function
{
	
	////printf("Inside Yielding Function");
	if(ready_queue->next != NULL)
	{
		MyThread* child = insertNode(ready_queue);
		////printf("\n Yielding at: %d to %d", ready_queue->id, ready_queue->next->id);
		ready_queue = ready_queue->next;
		swapcontext(&(child->context),&ready_queue->context);
	}
	
}

MySemaphore MySemaphoreInit(int initialValue)
{
	if(initialValue < 0)
	{
		//printf("Didn't create a semaphore. Improper Initial Value");
		return;
	}
	
	MySemaphore *sem = malloc(sizeof(MySemaphore)); 
	sem->semaphoreId = semCount++;
	sem->semaphoreValue = initialValue;

	insertIntoSemaphoreQueue(sem);
	
	return *sem;
	
}

int HasWaitingThreads(MySemaphore sem)
{
	MyThread* trav = semaphore_blocked_queue;
	
	while(trav != NULL)
	{
		if(trav->semaphoreBlocked == sem.semaphoreId)
			return 1;
		trav = trav->next;
	}
	
	return 0;
}

int MySemaphoreDestroy(MySemaphore sem)
{
	//printf("Inside Semaphore Destroy\n");
	if(HasWaitingThreads(sem))
	{
		//printf("\nCan't delete, waiting threads");
		return -1;
	}	
	if(deleteSemaphore(sem.semaphoreId))
	{
		//printf("\nDeleted");
		return 0;
	}
	else
	{
		//printf("\nCan't delete, unknown reason");
		return -1;
	}
	
}

void RemoveFromSemaphoreBlocked(MySemaphore* sem)
{
	MyThread* trav = semaphore_blocked_queue;
	MyThread* temp;
	//printf("CP1");
	if(semaphore_blocked_queue == NULL)
		return;

	//printf("CP2");
	//printf("\n Blocked Queue Head Waiting on: %d, Signal got for sem: %d", semaphore_blocked_queue->semaphoreBlocked, sem->semaphoreId);
	if(semaphore_blocked_queue->semaphoreBlocked == sem->semaphoreId)
	{
		//printf("CP3");
		temp = semaphore_blocked_queue->next;
		insertIntoReadyQueue(semaphore_blocked_queue);
		//printf("Yayyy! Unblocked through signal, Thread Id: %d\n", semaphore_blocked_queue->id);
		semaphore_blocked_queue = temp;
		return;
	}

	while(trav->next != NULL)
	{
		if(trav->next->semaphoreBlocked == sem->semaphoreId)
		{
			temp = trav->next->next;
			insertIntoReadyQueue(trav->next);
			trav->next = temp;
		}
		trav = trav->next;
	}
	
}


void MySemaphoreSignal(MySemaphore semaphore)
{

	/*The queue of S has no waiting thread 
The counter of S is increased by one and the thread resumes its execution.
The queue of S has waiting threads 
In this case, the counter of S must be zero (see the discussion of Wait above). One of the waiting threads will be allowed to leave the queue and resume its execution. The thread that executes Signal also continues.*/

	MySemaphore* sem = getSemaphoreFromId(semaphore.semaphoreId);
	sem->semaphoreValue++;
	if(sem->semaphoreValue <= 0)
	{
		if(!HasWaitingThreads(*sem))
		{
			//printf("I don't see any threads waiting");
			return;
		}
		else
		{
			//printf("Yes, there are some threads waiting");
			RemoveFromSemaphoreBlocked(sem); //remove the first thread you find with the same blockedSemaphoreId and add to ready queue.
		}
	}
	return;
}





void MySemaphoreWait(MySemaphore semaphore)
{

	/*The counter of S is positive 
In this case, the counter is decreased by 1 and the thread resumes its execution.
The counter of S is zero 
In this case, the thread is suspended and put into the private queue of S.*/
	
	MySemaphore* sem = getSemaphoreFromId(semaphore.semaphoreId);
	//capture the context here (1)
	//printf("\nTP1");
	ucontext_t currentContext;
	getcontext(&currentContext);

	//printf("\nTP1");
	//printf("\n Semaphore Id: %d", sem->semaphoreId);
	//printf("\n Semaphore Value: %d", sem->semaphoreValue);
	
	sem->semaphoreValue--;
	if (sem->semaphoreValue >= 0) //Verify once, if Semaphore value change from 1 to 0 allows the execu
		return; //the semaphore is free for access

	//printf("\nTP2");
	printQueues();
	//printf("\n Semaphore Value: %d", sem->semaphoreValue);
	printQueues();
	if(ready_queue == NULL)
	{
		setcontext(&controller);
	}
	if (ready_queue != NULL)
	{
		//printf("\nTP2");
		//printf("\nTP2");
		ready_queue->semaphoreBlocked = sem->semaphoreId;
		MyThread* trav = ready_queue->next;
		MyThread* temp = ready_queue;
		//printf("\nTP3");
		insertIntoSemaphoreBlockedQueue(ready_queue);
		ready_queue->context = currentContext;
		ready_queue->next = NULL;
		ready_queue = trav;
		////printf("\n Printing the Queues from Thread Join: ");
		//printQueues();
		if(ready_queue != NULL)
			setcontext(&ready_queue->context);

		//Put the context captured at (1) into the blockedSemaphoreQueue
		
	}
}






void CheckForUnblocking()
{

	int isReadyEmpty = 0;
	if(ready_queue == NULL)
		isReadyEmpty = 1;

	//printf("\nStarting Checking of Unblocking");
	int i;
	int flag = 0;
	printQueues(); //getting segmentation fault here, need to check further - only for the last blocked thread.
	//printf("CP1");
	if(blocked_queue == NULL)
		return;
	//printf("CP2");
	MyThread* trav = blocked_queue;
	MyThread* temp;

		flag = 0;
		for(i=0; i<ARR_SIZE; i++)
		{
			if(trav->blockedFor[i] == 0)
				break;
			if(presentInReadyQueue(trav->blockedFor[i]))
			{
				flag = 1;
				//printf("\nFlagged 1");
				break;
			}
			if(presentInSemaphoreBlockedQueue(trav->blockedFor[i]))
			{
				//printQueues();
				flag = 1;
				//printf("\nFlagged 1");
				//printQueues();

				break;
			}	
		}
		//printf("CP3");
		if(flag == 0)
		{
			blocked_queue = blocked_queue->next;
			//printf("\n Before inserting to ready queue: %d", trav->id);
			insertIntoReadyQueue(trav);
			if(blocked_queue == NULL)
			{
				//printf("\nUnblocking Last Node at Head: %d", trav->id);
				if(isReadyEmpty == 1)
				{
					//printf("Yayy! it works!");
					setcontext(&ready_queue->context);
				}
				return;			
			}			
		}

	
		
	MyThread* prev = blocked_queue;
	trav = blocked_queue;
	while(trav != NULL)
	{
		flag = 0;
		for(i=0; i<ARR_SIZE; i++)
		{
			if(trav->blockedFor[i] == 0)
				break;
			if(presentInReadyQueue(trav->blockedFor[i]))
			{
				flag = 1;
				break;
			}	
			if(presentInSemaphoreBlockedQueue(trav->blockedFor[i]))
			{
				//printQueues();
				flag = 1;
				//printf("\nFlagged 1");
				//printQueues();

				break;
			}
		}
		if(flag == 0)
		{
			////printf("Before Inserting - id: %d", trav->id);
			temp = trav;
			if(prev == trav)
			{
				blocked_queue = blocked_queue->next;
				prev = blocked_queue;
				trav = blocked_queue;
			}
			else
			{
				trav = trav->next;
			}
			trav->next = trav->next->next;
			insertIntoReadyQueue(temp);
			////printf("Inserted Node");
			////printf("\nUnblocking Node: %d", temp->id);
			
		}
		else
		{
			prev = trav;
			trav = trav->next;
		}
			
	}

	if(isReadyEmpty == 1)
	{
		//printf("Yayy! it works!");
		setcontext(&ready_queue->context);
	}
	
}

void MyThreadExit(void)
{
	setcontext(&controller);
}




void MyThreadInit(void(*start_funct)(void *), void *args)
{
start = 1;
	
	MyThread* node;
    	////printf("\nCP1");
	ready_queue = malloc(sizeof(MyThread));
	getcontext(&ready_queue->context);
 	ready_queue->context.uc_link=&controller;
 	ready_queue->context.uc_stack.ss_sp=malloc(MEM);
 	ready_queue->context.uc_stack.ss_size=MEM;
 	ready_queue->context.uc_stack.ss_flags=0;
	ready_queue->id = threadCount++;
	////printf("Allocation Initial Id: %d", ready_queue->id);

	makecontext(&ready_queue->context, (void*)start_funct, 1, args);

	ucontext_t next;
	int id;	

	getcontext(&controller);

	////printf("\nAbout to Start Checking of Unblocking");
	//printQueues();
	CheckForUnblocking();
	////printf("\nChecked for Block");

	
	//printQueues();


	if(ready_queue != NULL)
	{
		
		
		if(start == 1)
		{
			next = ready_queue->context;
			id = ready_queue->id;
			////printf("\n 2 Should Repeat:");
			////printf("\n4Allocation Initial Id: %d", ready_queue->id);
			start = 0;
			//printQueues();
			setcontext(&next);
		}
		else if (ready_queue->next != NULL)
		{	
			ready_queue = ready_queue->next;
			//CheckForUnblocking();
			next = ready_queue->context;
			id = ready_queue->id;
			////printf("\n 2 Should Repeat:");
			////printf("\nSetting Context Id: %d",id);


			//printQueues();

			setcontext(&next);
		}
		else 
		{	
			////printf("1Entering New Code");
			ready_queue = NULL;
			////printf("2Entering New Code");
			//printQueues();
			CheckForUnblocking();
			////printf("1Entering New Code");
			if (ready_queue != NULL)
			{
				next = ready_queue->context;
				id = ready_queue->id;
				////printf("\n 2 Should Repeat:");
				////printf("\nSetting Context Id: %d",id);


				//printQueues();

				setcontext(&next);
			}
		}
	}


}

