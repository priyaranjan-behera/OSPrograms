#include<stdio.h>
#include<pthread.h>

typedef struct thread_info{
int thread_id;
} thread_info;



void *thread_func(void *arg)
{
    int k;
    pthread_yield();

    
    thread_info *data = (thread_info*) arg;
    int i = 0;
    while(i < 100)
    {
        printf("Hello from : %d %d\n",data->thread_id, i);
        i++;
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t threads[10];
    thread_info threads_info[10];
    threads_info[0].thread_id = 1;  
    int rc;
    if (rc = pthread_create(&threads[0], NULL, thread_func, &threads_info[0]))
    {
        fprintf(stderr, "Error in pthread create, rc: %d \n", rc);
        return 1;
    }
    

    
    int i = 0;
    
    while(i<100)
    {
        printf("Hello from main %d\n", i);
        i++;
    }
    
    pthread_join(threads[0], NULL);
    
    return 0;
}


