#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#define MEM 8000
#define ARR_SIZE 10

int threadCount = 0;
int start;

typedef struct MyThread {
    ucontext_t context;
    int id;
    struct MyThread * next;
    struct MyThread * prev;
    int parent;
    int blockedFor[ARR_SIZE];
} MyThread;

MyThread* ready_queue;
MyThread* blocked_queue;

MyThread controller;

MyThread child1, child2;
MyThread trial;

void printQueues()
{
	MyThread* trial;
	int cnt;
//
		trial = ready_queue;
		cnt = 0;
		//printf("\n*********************************");
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
			printf(" %d ", trial->id);
			trial = trial->next;
			cnt++;
		}


		printf("\nThe Total Count of Thread in Block_Queue is %d", cnt);
		printf("\n*********************************");
		//

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
		trav->next->prev = trav;
		return trav->next;
	}

}

void insertIntoBlockedQueue(MyThread* newNode)
{

	//printf("\n Inserting to Block Queue. Thread: %d", newNode->id);
	//printf("\n This Thread is waiting on: ");
	int i;
	for (i=0; i<ARR_SIZE; i++)
	{
		//printf(" %d ", newNode->blockedFor[i]);
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

void insertIntoReadyQueue(MyThread* newNode)
{

	//printf("\n Inserting to Ready Queue. Thread: %d", newNode->id);
	//printf("\nChecking if ready_queue is NULL");

	if(ready_queue == NULL)
	{
		ready_queue = newNode;
		newNode->next = NULL;
		//printf("Ready Queue was null");
		return;
	}
	
	MyThread* trav;
	trav = ready_queue;
	while(trav->next != NULL) // make changes so as to create a 
		trav = trav->next;
	trav->next = newNode;
	trav->next->next = NULL;
	
}

int MyThreadJoin(MyThread thread)
{
	if (thread.parent != ready_queue->id)
		return -1; 
	if (!presentInReadyQueue(thread.id)) 
		return 1;

	memset(&(ready_queue->blockedFor[0]), 0, sizeof((ready_queue->blockedFor)));
	ready_queue->blockedFor[0] = thread.id;
	MyThread* trav = ready_queue->next;
	MyThread* temp = ready_queue;
	insertIntoBlockedQueue(ready_queue);
	ready_queue->next = NULL;
	ready_queue = trav;
	//printf("\n Printing the Queues from Thread Join: ");
	//printQueues();
	swapcontext(&temp->context, &ready_queue->context);

	return 1;
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
	
	//printf("\nThe counter returns %d", counter);
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
	//printf("\n Printing the Queues from Thread Join: ");
	//printQueues();
	swapcontext(&temp->context, &ready_queue->context);


}

int presentInReadyQueue(int id)
{
	MyThread* trav;
	trav = ready_queue;
	while(trav!=NULL)
	{
		if(trav->id == id)
			return 1;
		trav = trav->next;
	}
	return 0;
}


MyThread* moveNextNode(MyThread *head)
{
	head = head->next;
	head->prev = NULL;
}

MyThread MyThreadCreate (void(*start_funct)(void *), void *args) //Finalizes Create Thread Function
{
	MyThread* child = insertNode(ready_queue);
	getcontext(&(child->context));
	child->context.uc_stack.ss_sp=malloc(MEM);
 	child->context.uc_stack.ss_size=MEM;
 	child->context.uc_stack.ss_flags=0;
	child->id = threadCount++;
	child->parent = ready_queue->id;
	child->context.uc_link=&controller;
	makecontext(&(child->context), (void*)start_funct, 1, (int)args);
	//printf("\nHi, this is the %d thread getting created, my parent is: %d", child->id, ready_queue->id);
	//printQueues();
	return *child;
}

MyThreadYield() // Finalized Yield Function
{
	
	//printf("Inside Yielding Function");
	if(ready_queue->next != NULL)
	{
		MyThread* child = insertNode(ready_queue);
		//printf("\n Yielding at: %d to %d", ready_queue->id, ready_queue->next->id);
		ready_queue = ready_queue->next;
		swapcontext(&(child->context),&ready_queue->context);
	}
	
}

void CheckForUnblocking()
{
	//printf("\nStarting Checking of Unblocking");
	int i;
	int flag = 0;
	//printQueues(); //getting segmentation fault here, need to check further - only for the last blocked thread.
	if(blocked_queue == NULL)
		return;
	
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
		}

		if(flag == 0)
		{
			blocked_queue = blocked_queue->next;
			//printf("\n Before inserting to ready queue: %d", trav->id);
			insertIntoReadyQueue(trav);
			if(blocked_queue == NULL)
			{
				//printf("\nUnblocking Last Node at Head: %d", trav->id);
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
		}
		if(flag == 0)
		{
			//printf("Before Inserting - id: %d", trav->id);
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
			//printf("Inserted Node");
			//printf("\nUnblocking Node: %d", temp->id);
			
		}
		else
		{
			prev = trav;
			trav = trav->next;
		}
			
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
    	//printf("\nCP1");
	ready_queue = malloc(sizeof(MyThread));
	getcontext(&ready_queue->context);
 	ready_queue->context.uc_link=&controller;
 	ready_queue->context.uc_stack.ss_sp=malloc(MEM);
 	ready_queue->context.uc_stack.ss_size=MEM;
 	ready_queue->context.uc_stack.ss_flags=0;
	ready_queue->id = threadCount++;
	//printf("Allocation Initial Id: %d", ready_queue->id);

	makecontext(&ready_queue->context, (void*)start_funct, 1, (int)args);

	ucontext_t next;
	int id;	

	getcontext(&controller);

	//printf("\nAbout to Start Checking of Unblocking");
	//printQueues();
	CheckForUnblocking();
	//printf("\nChecked for Block");

	
	//printQueues();


	if(ready_queue != NULL)
	{
		
		
		if(start == 1)
		{
			next = ready_queue->context;
			id = ready_queue->id;
			//printf("\n 2 Should Repeat:");
			//printf("\n4Allocation Initial Id: %d", ready_queue->id);
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
			//printf("\n 2 Should Repeat:");
			//printf("\nSetting Context Id: %d",id);


			//printQueues();

			setcontext(&next);
		}
		else 
		{	
			//printf("1Entering New Code");
			ready_queue = NULL;
			//printf("2Entering New Code");
			//printQueues();
			CheckForUnblocking();
			//printf("1Entering New Code");
			if (ready_queue != NULL)
			{
				next = ready_queue->context;
				id = ready_queue->id;
				//printf("\n 2 Should Repeat:");
				//printf("\nSetting Context Id: %d",id);


				//printQueues();

				setcontext(&next);
			}
		}
	}
 	//printf("completed\n");

}

