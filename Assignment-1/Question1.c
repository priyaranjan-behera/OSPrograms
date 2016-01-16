#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>

typedef struct thread_info{
int thread_id;
} thread_info;

typedef struct MyThread{
pthread_t* thread;
thread_info* info;
} MyThread;

void MyThreadYield(void)
{
    pthread_yield();
}

void *thread_func(void *arg)
{
    int k;
    MyThreadYield();
    thread_info *data = (thread_info*) arg;
    int i = 0;
    while(i < 100)
    {
        printf("Hello from : %d %d\n",data->thread_id, i);
        i++;
    }
    pthread_exit(NULL);
}



MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
{
    pthread_t thread;
    thread_info info;
    info.thread_id = rand();
    int rc;
    
    MyThread ret;
    ret.thread = &thread;
    ret.info = &info;
    rc = (pthread_create(&thread, NULL, start_funct, args));
    if (rc)
    {
        fprintf(stderr, "Error in pthread create, rc: %d \n", rc);
    }
   
    return ret;
}

int main(int argc, char **argv)
{
    thread_info info;
    info.thread_id = 10;
    
    MyThread thread = MyThreadCreate(thread_func, &info);
    
    int i = 0;
    
    while(i<100)
    {
        printf("Hello from main %d\n", i);
        i++;
    }
    
    return 0;
}


