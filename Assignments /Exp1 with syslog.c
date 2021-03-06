#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#include<syslog.h>
#define MEM 8000
#define ARR_SIZE 10

int count = 0;
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
		syslog(LOG_INFO, "\n*********************************");
		syslog(LOG_INFO, "\nPrinting Ready Queue at Start: ");
		while(trial != NULL)
		{	
			syslog(LOG_INFO, " %d ", trial->id);
			trial = trial->next;
			cnt++;
		}


		syslog(LOG_INFO, "\nThe Total Count of Thread in Ready_Queue is %d", cnt);
		syslog(LOG_INFO, "\n*********************************");

		trial = blocked_queue;
		cnt = 0;
		syslog(LOG_INFO, "\n*********************************");
		syslog(LOG_INFO, "\nPrinting Block Queue at Start: ");
		while(trial != NULL)
		{	
			syslog(LOG_INFO, " %d ", trial->id);
			trial = trial->next;
			cnt++;
		}


		syslog(LOG_INFO, "\nThe Total Count of Thread in Block_Queue is %d", cnt);
		syslog(LOG_INFO, "\n*********************************");
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

	syslog(LOG_INFO, "\n Inserting to Block Queue. Thread: %d", newNode->id);
	syslog(LOG_INFO, "\n This Thread is waiting on: ");
	int i;
	for (i=0; i<ARR_SIZE; i++)
	{
		syslog(LOG_INFO, " %d ", newNode->blockedFor[i]);
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

	syslog(LOG_INFO, "\n Inserting to Ready Queue. Thread: %d", newNode->id);
	syslog(LOG_INFO, "\nChecking if ready_queue is NULL");

	if(ready_queue == NULL)
	{
		ready_queue = newNode;
		syslog(LOG_INFO, "Ready Queue was null");
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
	syslog(LOG_INFO, "\n Printing the Queues from Thread Join: ");
	printQueues();
	swapcontext(&temp->context, &ready_queue->context);

	return 1;
}

void MyThreadJoinAll(void)
{
	MyThread* trav = ready_queue;
	int children[ARR_SIZE];
	
	int i, counter = 0;
	while(trav != NULL)
	{
		if(trav->parent == ready_queue->id)
		{
			children[counter++] = trav->id;
		}
	}
	
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
	syslog(LOG_INFO, "\n Printing the Queues from Thread Join: ");
	printQueues();
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
	child->id = count++;
	child->parent = ready_queue->id;
	child->context.uc_link=&controller;
	makecontext(&(child->context), (void*)start_funct, args);
	syslog(LOG_INFO, "\nHi, this is the %d thread getting created, my parent is: %d", child->id, ready_queue->id);
	printQueues();
	return *child;
}

MyThreadYield() // Finalized Yield Function
{
	MyThread* child = insertNode(ready_queue);
	syslog(LOG_INFO, "Inside Yielding Function");
	if(ready_queue->next != NULL)
	{
		syslog(LOG_INFO, "\n Yielding at: %d to %d", ready_queue->id, ready_queue->next->id);
		getcontext(&(child->context));
		ready_queue = ready_queue->next;
		setcontext(&ready_queue->context);
	}
	
}

void CheckForUnblocking()
{
	syslog(LOG_INFO, "\nStarting Checking of Unblocking");
	printQueues();
	syslog(LOG_INFO, "TP1"); //getting segmentation fault here, need to check further - only for the last blocked thread.
	int i;
	int flag = 0;
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
				syslog(LOG_INFO, "Flagged 1");
				break;
			}	
		}

		if(flag == 0)
		{
			blocked_queue = trav->next;
			syslog(LOG_INFO, "\n Before inserting to ready queue: %d", blocked_queue->id);
			insertIntoReadyQueue(trav);
			if(blocked_queue == NULL)
			{
				syslog(LOG_INFO, "\nUnblocking Node at Head: %d", trav->id);
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
			syslog(LOG_INFO, "Before Inserting - id: %d", trav->id);
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
			syslog(LOG_INFO, "Inserted Node");
			//syslog(LOG_INFO, "\nUnblocking Node: %d", temp->id);
			
		}
		else
		{
			prev = trav;
			trav = trav->next;
		}
			
	}
	
}


void fn2()
{
 printf("\nExecuting F2, ready queue id: %d\n", ready_queue->id);
}

int fn1()
{
 printf("\nExecuting F1, ready queue id: %d\n", ready_queue->id);
 child2 = MyThreadCreate((void*)fn2, NULL);
child2 = MyThreadCreate((void*)fn2, NULL);
 printf("\nthis is from 1_1\n");
 syslog(LOG_INFO, "\n Joining the threads: parent %d(won't run again), child %d", ready_queue->id, child2.id);
 MyThreadJoin(child2);
 printf("\nthis is from 1_2\n");
 syslog(LOG_INFO, "THIS SHOULD NOT PRINT: If printed, unsuccessful");
}


int main(int argc, char *argv[])
{

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog ("exampleprog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);


	start = 1;
	
	MyThread* node;
    	syslog(LOG_INFO, "\nCP1");
	ready_queue = malloc(sizeof(MyThread));
	getcontext(&ready_queue->context);
 	ready_queue->context.uc_link=&controller;
 	ready_queue->context.uc_stack.ss_sp=malloc(MEM);
 	ready_queue->context.uc_stack.ss_size=MEM;
 	ready_queue->context.uc_stack.ss_flags=0;
	ready_queue->id = count++;
	syslog(LOG_INFO, "Allocation Initial Id: %d", ready_queue->id);

	makecontext(&ready_queue->context, (void*)&fn1, 0);

	child1 = MyThreadCreate((void*)fn1, NULL);
	child2 = MyThreadCreate((void*)fn2, NULL);

	syslog(LOG_INFO, "\nPrinting the entire list: ");

	ucontext_t next;
	int id;	
	
	syslog(LOG_INFO, "\n3Allocation Initial Id: %d", ready_queue->id);

	getcontext(&controller);
	syslog(LOG_INFO, "\nShould Repeat:");
	syslog(LOG_INFO, "\nAbout to Start Checking of Unblocking");
	printQueues();
	CheckForUnblocking();
	syslog(LOG_INFO, "\nChecked for Block");

	
	syslog(LOG_INFO, "\n3Allocation Initial Id: %d", ready_queue->id);



	if(ready_queue != NULL)
	{
		
		
		if(start == 1)
		{
			next = ready_queue->context;
			id = ready_queue->id;
			syslog(LOG_INFO, "\n 2 Should Repeat:");
			syslog(LOG_INFO, "\n4Allocation Initial Id: %d", ready_queue->id);
			start = 0;
			printQueues();
			setcontext(&next);
		}
		else if (ready_queue->next != NULL)
		{	
			ready_queue = ready_queue->next;
			//CheckForUnblocking();
			next = ready_queue->context;
			id = ready_queue->id;
			syslog(LOG_INFO, "\n 2 Should Repeat:");
			syslog(LOG_INFO, "\nSetting Context Id: %d",id);


			printQueues();

			setcontext(&next);
		}
		else 
		{
			syslog(LOG_INFO, "1Entering New Code");
			ready_queue = NULL;
			syslog(LOG_INFO, "2Entering New Code");
			printQueues();
			CheckForUnblocking();
			syslog(LOG_INFO, "1Entering New Code");
			if (ready_queue != NULL)
			{
				next = ready_queue->context;
				id = ready_queue->id;
				syslog(LOG_INFO, "\n 2 Should Repeat:");
				syslog(LOG_INFO, "\nSetting Context Id: %d",id);


				printQueues();

				setcontext(&next);
			}
		}
	}
 	syslog(LOG_INFO, "completed\n");
	closelog ();
 	exit(0);
}
