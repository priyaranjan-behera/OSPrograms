#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#define MEM 8000

int count = 0;

typedef struct MyThread {
    ucontext_t context;
    int id;
    struct MyThread * next;
    struct MyThread * prev;
    int parent;
    int blockedFor[10];
} MyThread;

MyThread* ready_queue;
MyThread* blocked_queue;

MyThread controller;

MyThread child1, child2;

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
	printf("\nHi, this is the %d thread getting created, my parent is: %d", count, ready_queue->id);
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


void fn2()
{
 printf("\nExecuting F2, ready queue id: %d\n", ready_queue->id);
}

int fn1()
{
 printf("\nExecuting F1, ready queue id: %d\n", ready_queue->id);
 child2 = MyThreadCreate((void*)fn2, NULL);
 printf("\nthis is from 1_1\n");
 MyThreadYield();
 printf("\nthis is from 1_2\n");
}


int main(int argc, char *argv[])
{
	MyThread* node;
    	printf("\nCP1");
	ready_queue = malloc(sizeof(MyThread));
	getcontext(&ready_queue->context);
 	ready_queue->context.uc_link=&controller;
 	ready_queue->context.uc_stack.ss_sp=malloc(MEM);
 	ready_queue->context.uc_stack.ss_size=MEM;
 	ready_queue->context.uc_stack.ss_flags=0;

	makecontext(&ready_queue->context, (void*)&fn1, 0);

	child2 = MyThreadCreate((void*)fn2, NULL);

	printf("\nCP2");
	child1 = MyThreadCreate((void*)fn1, NULL);
	child1 = MyThreadCreate((void*)fn1, NULL);
	child2 = MyThreadCreate((void*)fn2, NULL);

	printf("\nPrinting the entire list: ");

	MyThread* test = ready_queue;
	while (test->next != NULL)
	{
		printf("\nForward Id: %d", test->id);
		test = test->next;
	}

	while (test != NULL)
	{
		printf("\nBackward Id: %d", test->id);
		test = test->prev;
	}
	ucontext_t next;
	int id;	
	
	getcontext(&controller);
	printf("\nShould Repeat:");
	if(ready_queue != NULL)
	{
		ready_queue = ready_queue->next;
		if (ready_queue != NULL)
		{
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
