#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t pid,ppid;
    pid=getpid();
    ppid=getppid();
    printf("system call:pid=%d\n",pid);
    printf("system call:ppid=%d\n",ppid);
    return 0;
}