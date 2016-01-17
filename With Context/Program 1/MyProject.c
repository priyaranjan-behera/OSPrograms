#include<stdio.h>
#include<stdlib.h>
#include<ucontext.h>
#define MEM 64000
ucontext_t T1, T2,Main;
ucontext_t a;
ucontext_t next;
typedef struct MyThread {
ucontext_t context;
} MyThread;

MyThread child1, child2;

int fn1()
{
 ucontext_t current;
 printf("this is from 1\n");
}
void fn2()
{
 printf("this is from 2\n");
}
MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
	MyThread thread;
	getcontext(&thread.context);
	getcontext(&parent);
	thread.context.uc_link=&next;
 	thread.context.uc_stack.ss_sp=malloc(MEM);
 	thread.context.uc_stack.ss_size=MEM;
 	thread.context.uc_stack.ss_flags=0;
 	makecontext(&thread.context, (void*)start_funct, args);
	return thread;
}

void MyThreadYield(void)
{
	setcontext(&next);
}



int main(int argc, char *argv[])
{
 	getcontext(&Main);

	child1 = MyThreadCreate((void*)fn1, NULL);
	child2 = MyThreadCreate((void*)fn2, NULL);
	
	swapcontext(&next, &child.context);
 	printf("completed\n");
 	exit(0);
}
