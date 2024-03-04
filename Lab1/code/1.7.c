#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int var=0;
pthread_mutex_t mutex;

void* thread1(void* param){
    for(int i=0;i<5000;i++){
        pthread_mutex_lock(&mutex);
        var+=2;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}
void* thread2(void* param){
    for(int i=0;i<5000;i++){
        pthread_mutex_lock(&mutex);
        var-=1;
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

int main(int argc, char* argv[]){
    pthread_t tid1,tid2;
    pthread_attr_t attr1,attr2;

    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_create(&tid1,&attr1,thread1,argv[1]);
    pthread_create(&tid2,&attr2,thread2,argv[1]);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);

    printf("var=%d\n",var);

    return 0;
}