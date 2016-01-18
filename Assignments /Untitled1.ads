#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#define MEM 64000
ucontext_t T1, T2,Main;
ucontext_t a;
ucontext_t next;
typedef struct MyThread {
ucontext_t context;
int id;
} MyThread;

int count = 0;

MyThread thread;



typedef struct node {
    ucontext_t context;
    int id;
    struct node * next;
    struct node * prev;
    int parent;
    int blockedFor[10];
} node;

node* ready_queue = NULL;
node* blocked_queue = NULL;

node child1, child2;

node* insertNode(node* head)
{
	node* trav;
	trav = head;
	if(head == NULL)
	{
		head = malloc(sizeof(node));
		return head;
	}
	else
	{
		while(trav->next != NULL)
			trav = trav-> next;

		trav->next = malloc(sizeof(node));
		trav->next->prev = trav;
		return trav->next;
	}

}

node* moveNextNode(node *head)
{
	head = head->next;
	head->prev = NULL;
}





int fn1()
{
 printf("this is from 1_1\n");
 printf("this is from 1_2\n");
}
void fn2()
{
 printf("this is from 2\n");
}
node MyThreadCreate (void(*start_funct)(void *), void *args)
{
	node* child = insertNode(ready_queue);
	ucontext_t current;
	getcontext(&(child->context));
	getcontext(&current);
	child->context.uc_stack.ss_sp=malloc(MEM);
 	child->context.uc_stack.ss_size=MEM;
 	child->context.uc_stack.ss_flags=0;
	child->id = count++;
	child->parent = ready_queue->id;
	child->context.uc_link=&(child->prev->context);
	makecontext(&(child->context), (void*)start_funct, args);
	return *child;
}




int main(int argc, char *argv[])
{
    printf("CP1");
	child2 = MyThreadCreate((void*)fn2, NULL);
	printf("CP2");
	child1 = MyThreadCreate((void*)fn1, NULL);

	printf("Printing the entire list: ");

	node* test = ready_queue;
	while (test->next != NULL)
	{
		printf("\nId: %d", test->id);
		test = test->next;
	}

	while (test != NULL)
	{
		printf("\nId: %d", test->id);
		test = test->prev;
	}

	swapcontext(&next, &child1.context);
 	printf("completed\n");
 	exit(0);
}
