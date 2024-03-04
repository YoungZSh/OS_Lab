#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid(); 
    pthread_t tid = pthread_self();

    printf("exec_prog:pid= %d\n", pid);
    printf("exec_prog:tid= %ld\n", tid);

    return 0;
}