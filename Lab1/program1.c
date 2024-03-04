#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid(); 
    pthread_t tid = pthread_self();

    printf("system call:pid= %d\n", pid);
    printf("system call:tid= %ld\n", tid);

    return 0;
}