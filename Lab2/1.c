#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
int flag = 0;
void inter_handler()
{
    // TODO
    flag=1;
}
void waiting()
{ 
    // TODO
    //sleep(5);
    alarm(5);
    pause();
}
void nothing(){}
int main()
{
    // TODO: 五秒之后或接收到两个信号
    signal(SIGINT, inter_handler);
    signal(SIGQUIT, inter_handler);
    signal(SIGALRM, inter_handler);
    pid_t pid1 = -1, pid2 = -1;
    while (pid1 == -1)
        pid1 = fork();
    if (pid1 > 0)
    {
        while (pid2 == -1)
            pid2 = fork();
        if (pid2 > 0)
        {
            // TODO: 父进程
            //signal(SIGINT, inter_handler);
            //signal(SIGQUIT, inter_handler);
            sleep(1);//确保子进程进入pause()状态
            waiting();
            if(flag){
                kill(pid1,16);
                kill(pid2,17);
                wait(NULL);
                wait(NULL);
                printf("\nParent process is killed!!\n");
                exit(0);
            }
        }
        else
        {
            // TODO: 子进程 2
            signal(17,nothing);
            pause();
            printf("\nChild process 2 is killed by parent!!\n");
            exit(0);
        }
    }
    else
    {
        // TODO：子进程 1
        signal(16,nothing);
        pause();
        printf("\nChild process 1 is killed by parent!!\n");
        exit(0);
    }
    return 0;
}