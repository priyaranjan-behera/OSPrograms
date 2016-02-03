#include <stdio.h>
#include<stdlib.h>
#include "mythread.h"


MySemaphore sem1,sem2;

n=0;

struct assembly{
    int array[5000];
    int pointer;    
    int object;
};

struct assembly belt1,belt2;
int exit1 = 1, exit2 = 1;

void producer(void *x){
    
    int id =(int) x;
    int choice;
    struct assembly* belt = id == 1 ? &belt1 : &belt2;
    MySemaphore sem = id == 1 ? sem1 : sem2;
    if(id == 1) exit1 = 0;
    if(id == 2) exit2 = 0;
    while(n < 100){
        n++;
        choice  = rand()%10;
        if(choice < 5){
            belt->array[belt->pointer%5000] = n;
            belt->pointer+=1;    
            MySemaphoreSignal(sem);
        }
        else{
            printf("ThreadYield : producer : %d\n",id);
            MyThreadYield();
        }
    }
    if(id == 1) exit1 = 1;
    if(id == 2) exit2 = 1;
    MySemaphoreSignal(sem);
    MyThreadExit();
}

void consumer(void* x){
    int id =(int) x;
    int choice,location;
    int exit;
    struct assembly* belt = id == 1 ? &belt1 : &belt2;
    MySemaphore sem = id == 1 ? sem1 : sem2;
    exit = id == 1 ? exit1 : exit2;
    while(1){
        MySemaphoreWait(sem);
        location = (belt->pointer-belt->object+5000)%5000;
        printf("Consuming at location : %d\n",location);
        if(belt->object > 0)
            belt->object -= 1;
        exit = id == 1 ? exit1 : exit2;
        if(belt->object == 0 && exit == 1){
            break;
        }
    }
    MySemaphoreDestroy(sem);
    MyThreadExit();
}


void t0(void * dummy)
{
  
  printf("t0 start\n");
  sem1=MySemaphoreInit(0);
  sem2=MySemaphoreInit(0);  
  printf("Sem1 = %d\n",(int)sem1);
  printf("Sem2 = %d\n",(int)sem2);
  MyThreadCreate(producer,(void*)1);
  MyThreadCreate(producer,(void*)2);
  MyThreadCreate(consumer,(void*)1);
  MyThreadCreate(consumer,(void*)2);
  MyThreadYield();
  MyThreadExit();
}


int main()
{
    
    belt1.object=0;
    belt1.pointer=0;
    belt2.pointer=0;
    belt2.object=0;
    MyThreadInit(t0, NULL);
}