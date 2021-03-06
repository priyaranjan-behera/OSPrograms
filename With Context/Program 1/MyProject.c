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
    struct node * child;

} node;

node* head = NULL;
node* cur = NULL;

node* getNewNode()
{
	node* ret;
	if (head == NULL)
	{
		head = malloc(sizeof(node));
		ret = head;
	}
	else
	{
		node* trav = head;
		while(trav->next != NULL)
		{
			trav = trav->next;
			printf("Traversing");
		}	
		trav->next = malloc(sizeof(node));
		ret = trav->next;
		ret->prev = trav;
		printf("Prev set");
		
	}
	
	return ret;

}


node child1, child2;

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
	node* child = getNewNode();
	getcontext(&(child->context));
	child->context.uc_stack.ss_sp=malloc(MEM);
 	child->context.uc_stack.ss_size=MEM;
 	child->context.uc_stack.ss_flags=0;
	child->id = count++;
	if (child-> prev != NULL)
	{
		thread.context.uc_link=&(child->prev->context);
		printf("The Previous Context is Initialized\n");
		printf("Present Id = %d ", child->id);
		printf("Previous Context Id: %d \n", child->prev->id);
	} 	
	
 	makecontext(&(child->context), (void*)start_funct, args);
	return *child;
}

void MyThreadYield(void)
{
	setcontext(&next);
}



int main(int argc, char *argv[])
{
	head = malloc(sizeof(node)); 
	head->id = count++;
 	getcontext(&(head->context));

	child2 = MyThreadCreate((void*)fn2, NULL);
	child1 = MyThreadCreate((void*)fn1, NULL);
	
	printf("Printing the entire list: ");
	node* test = head;
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
