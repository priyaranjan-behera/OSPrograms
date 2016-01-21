#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
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

	printf("\n Inserting to Block Queue. Thread: %d", newNode->id);
	printf("\n This Thread is waiting on: ");
	int i;
	for (i=0; i<ARR_SIZE; i++)
	{
		printf(" %d ", newNode->blockedFor[i]);
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

	printf("\n Inserting to Ready Queue. Thread: %d", newNode->id);
	int i;	
	if(ready_queue == NULL)
	{
		ready_queue = newNode;
		return;
	}
	
	MyThread* trav;
	trav = ready_queue;
	while(trav->next != NULL)
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

	MyThread* child = insertNode(blocked_queue); //modify code to insert into blocked queue directly.***** CHANGE CHANGEE
	getcontext(&(child->context));
	memset(&(child->blockedFor[0]), 0, sizeof((ready_queue->blockedFor)));
	ready_queue->blockedFor[0] = thread.id;
	insertIntoBlockedQueue(ready_queue);
	MyThread* trav = ready_queue;
	ready_queue = ready_queue->next;
	trav->next = NULL;
	swapcontext(&trav->context, &ready_queue->context);

	return 1;
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
	printf("\nHi, this is the %d thread getting created, my parent is: %d", child->id, ready_queue->id);
	return *child;
}

MyThreadYield() // Finalized Yield Function
{
	MyThread* child = insertNode(ready_queue);
	printf("Inside Yielding Function");
	if(ready_queue->next != NULL)
	{
		printf("\n Yielding at: %d to %d", ready_queue->id, ready_queue->next->id);
		getcontext(&(child->context));
		ready_queue = ready_queue->next;
		setcontext(&ready_queue->context);
	}
	
}

void CheckForUnblocking()
{
	int i;
	int flag = 0;
	if(blocked_queue == NULL)
		return;
	MyThread* trav = blocked_queue;

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
			
			insertIntoReadyQueue(trav);
			if(trav->next == NULL)
			{
				printf("\nUnblocking Node at Head: %d", trav->id);
				blocked_queue = NULL;
				return;			
			}			
		}

	
		
	
	while(trav->next != NULL)
	{
		flag = 0;
		for(i=0; i<ARR_SIZE; i++)
		{
			if(trav->next->blockedFor[i] == 0)
				break;
			if(presentInReadyQueue(trav->next->blockedFor[i]))
			{
				flag = 1;
				break;
			}	
		}
		if(flag == 0)
		{
			
			insertIntoReadyQueue(trav->next);
			printf("\nUnblocking Node: %d", trav->next->id);
			trav->next = trav->next->next;
		}
		trav = trav->next;
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
 printf("\nthis is from 1_1\n");
 printf("\n Joining the threads: parent %d(won't run again), child %d", ready_queue->id, child2.id);
 MyThreadJoin(child2);
 printf("\nthis is from 1_2\n");
 printf("THIS SHOULD NOT PRINT: If printed, unsuccessful");
}


int main(int argc, char *argv[])
{

	start = 1;
	MyThread* node;
    	printf("\nCP1");
	ready_queue = malloc(sizeof(MyThread));
	getcontext(&ready_queue->context);
 	ready_queue->context.uc_link=&controller;
 	ready_queue->context.uc_stack.ss_sp=malloc(MEM);
 	ready_queue->context.uc_stack.ss_size=MEM;
 	ready_queue->context.uc_stack.ss_flags=0;
	ready_queue->id = count++;
	printf("Allocation Initial Id: %d", ready_queue->id);

	makecontext(&ready_queue->context, (void*)&fn1, 0);

	//child1 = MyThreadCreate((void*)fn1, NULL);
	//child2 = MyThreadCreate((void*)fn2, NULL);

	printf("\nPrinting the entire list: ");

	MyThread* test = ready_queue;
	while (test->next != NULL)
	{
		printf("\nForward Id: %d", test->id);
		test = test->next;
	}

	printf("\n2Allocation Initial Id: %d", ready_queue->id);

	while (test != NULL)
	{
		printf("\nBackward Id: %d", test->id);
		test = test->prev;
	}
	ucontext_t next;
	int id;	
	
	printf("\n3Allocation Initial Id: %d", ready_queue->id);

	getcontext(&controller);
	printf("\nShould Repeat:");
	//CheckForUnblocking();
	printf("\nChecked for Block");

	
	printf("\n3Allocation Initial Id: %d", ready_queue->id);

	if(ready_queue != NULL)
	{
		
		MyThread* trial = ready_queue;
		int cnt = 0;
		while(trial != NULL)
		{
			trial = trial->next;
			cnt++;
		}

		printf("\n*********************************");
		printf("\nThe Total Count of Thread in Ready_Queue is %d", cnt);
		printf("\n*********************************");
		if(start == 1)
		{
			next = ready_queue->context;
			id = ready_queue->id;
			printf("\n 2 Should Repeat:");
			printf("\n4Allocation Initial Id: %d", ready_queue->id);
			start = 0;
			setcontext(&next);
		}
		else if (ready_queue->next != NULL)
		{	
			ready_queue = ready_queue->next;
			next = ready_queue->context;
			id = ready_queue->id;
			printf("\n 2 Should Repeat:");
			printf("\nSetting Context Id: %d",id);
			setcontext(&next);
		}
	}
 	printf("completed\n");

 	exit(0);
}
