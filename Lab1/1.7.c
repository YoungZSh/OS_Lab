#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int var=0;
sem_t S;

void* thread1(void* param){
    for(int i=0;i<5000;i++){
        sem_wait(&S);
        var+=2;
        sem_post(&S);
    }
    pthread_exit(0);
}
void* thread2(void* param){
    for(int i=0;i<5000;i++){
        sem_wait(&S);
        var-=1;
        sem_post(&S);
    }
    pthread_exit(0);
}

int main(int argc, char* argv[]){
    pthread_t tid1,tid2;
    pthread_attr_t attr1,attr2;

    sem_init(&S,0,1);

    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_create(&tid1,&attr1,thread1,argv[1]);
    pthread_create(&tid2,&attr2,thread2,argv[1]);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);

    printf("var=%d\n",var);

    return 0;
}