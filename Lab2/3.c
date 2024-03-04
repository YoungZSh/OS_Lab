#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESS_NAME_LEN 32 /*进程名长度*/ 
#define MIN_SLICE 10 /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024                                   /*内存大小*/
#define DEFAULT_MEM_START 0                                     /*起始位置*/
/* 内存分配算法 */
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3
int mem_size = DEFAULT_MEM_SIZE; /*内存大小*/
int ma_algorithm = MA_FF;        /*当前分配算法*/
static int pid = 0;              /*初始 pid*/
int flag = 0;                   /*设置内存大小标志*/

/*描述每一个空闲块的数据结构*/
struct free_block_type
{
    int size;
    int start_addr;
    struct free_block_type* next;
};
/*指向内存中空闲块链表的首指针*/
struct free_block_type* free_block;

/*每个进程分配到的内存块的描述*/
struct allocated_block
{
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block* next;
};
/*进程分配内存块链表的首指针*/
struct allocated_block* allocated_block_head = NULL;

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* init_free_block(int mem_size)
{
    struct free_block_type* fb;
    fb = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if (fb == NULL)
    {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}



void display_menu();
int set_mem_size();
void set_algorithm();
void rearrange(int algorithm);
void rearrange_FF();
void rearrange_BF();
void rearrange_WF();
int new_process();
int allocate_mem(struct allocated_block* ab);
void kill_process();
int free_mem(struct allocated_block* ab);
int dispose(struct allocated_block* free_ab);
int display_mem_usage();
void do_exit();
struct allocated_block* find_process(int pid);

int main()
{
    char choice;
    pid = 0;
    free_block = init_free_block(mem_size); // 初始化空闲区
    while (1)
    {
        display_menu(); // 显示菜单
        fflush(stdin);//清空输入缓冲区
        choice = getchar(); // 获取用户输入
        switch (choice)
        {
        case '1':
            set_mem_size();
            break; // 设置内存大小
        case '2':
            set_algorithm();
            flag = 1; break; // 设置算法
        case '3':
            new_process();
            flag = 1; break; // 创建新进程
        case '4':
            kill_process();
            flag = 1; break; // 删除进程
        case '5':
            display_mem_usage();
            flag = 1; break; // 显示内存使用
        case '0':
            do_exit();
            exit(0);
            // 释放链表并退出 
        default: break;
        }
    }
}




/*显示菜单*/
void display_menu()
{
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("0 - Exit\n");
}
/*设置内存的大小*/
int set_mem_size()
{
    int size;
    if (flag != 0)
    { // 防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size =");
    scanf("%d", &size);
    if (size > 0)
    {
        mem_size = size;
        free_block->size = mem_size;
    }
    flag = 1;
    return 1;
}
/* 设置当前的分配算法 */
void set_algorithm()
{
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit \n");
    printf("\t3 - Worst Fit \n");
    scanf("%d", &algorithm);
    if (algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;
    // 按指定算法重新排列空闲区链表
    rearrange(ma_algorithm);
}
/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm)
{
    switch (algorithm)
    {
    case MA_FF:
        rearrange_FF();
        break;
    case MA_BF:
        rearrange_BF();
        break;
    case MA_WF:
        rearrange_WF();
        break;
    }
}
/*按 FF 算法重新整理内存空闲块链表*/
void rearrange_FF()
{
    //合并相邻内存
    struct free_block_type* node = free_block;
    struct free_block_type* nextnode = node->next;
    while (nextnode != NULL) {
        if (node->start_addr + node->size == nextnode->start_addr) {
            node->size = node->size + nextnode->size;
            node->next = nextnode->next;
            free(nextnode);
            nextnode = node->next;
            continue;
        }
        node = nextnode;
        nextnode = node->next;
    }
}
/*按 BF 算法重新整理内存空闲块链表*/
void rearrange_BF()
{
    struct free_block_type* node = free_block;
    struct free_block_type* nextnode = node->next;
    struct free_block_type* prenode = NULL;
    int num = 1;//记录链表长度
    //合并相邻内存
    while (nextnode != NULL) {
        if (node->start_addr + node->size == nextnode->start_addr) {
            node->size = node->size + nextnode->size;
            node->next = nextnode->next;
            free(nextnode);
            nextnode = node->next;
            continue;
        }
        node = nextnode;
        nextnode = node->next;
        num++;
    }
    //对空闲内存列表从小至大排序
    struct free_block_type* new_head = NULL;
    struct free_block_type* new_tail = NULL;
    struct free_block_type* min_node;
    struct free_block_type* pre_min_node;
    for (int i = 0; i < num; i++) {
        min_node = free_block;
        pre_min_node = NULL;
        node = free_block;
        prenode = NULL;
        while (node != NULL) {
            if (node->size < min_node->size) {
                min_node = node;
                pre_min_node = prenode;
            }
            prenode = node;
            node = node->next;
        }
        //将每次循环中的最小空闲内存连接到新的链表中
        if (new_head == NULL) {
            new_head = min_node;
            new_tail = min_node;
            if (pre_min_node != NULL) {
                pre_min_node->next = min_node->next;
            }
            else {
                free_block = min_node->next;
            }
        }
        else {
            new_tail->next = min_node;
            new_tail = min_node;
            if (pre_min_node != NULL) {
                pre_min_node->next = min_node->next;
            }
            else {
                free_block = min_node->next;
            }
        }
    }
    new_tail->next = NULL;
    free_block = new_head;
}
/*按 WF 算法重新整理内存空闲块链表*/
void rearrange_WF()
{
    struct free_block_type* node = free_block;
    struct free_block_type* nextnode = node->next;
    struct free_block_type* prenode = NULL;
    int num = 1;//记录链表长度
    //合并相邻内存
    while (nextnode != NULL) {
        if (node->start_addr + node->size == nextnode->start_addr) {
            node->size = node->size + nextnode->size;
            node->next = nextnode->next;
            free(nextnode);
            nextnode = node->next;
            continue;
        }
        num++;
        node = nextnode;
        nextnode = node->next;
    }
    //对空闲内存列表从小至大排序
    struct free_block_type* new_head = NULL;
    struct free_block_type* new_tail = NULL;
    struct free_block_type* max_node;
    struct free_block_type* pre_max_node;
    for (int i = 0; i < num; i++) {
        max_node = free_block;
        pre_max_node = NULL;
        node = free_block;
        prenode = NULL;
        while (node != NULL) {
            if (node->size > max_node->size) {
                max_node = node;
                pre_max_node = prenode;
            }
            prenode = node;
            node = node->next;
        }
        //将每次循环中的最大空闲内存连接到新的链表中
        if (new_head == NULL) {
            new_head = max_node;
            new_tail = max_node;
            if (pre_max_node != NULL) {
                pre_max_node->next = max_node->next;
            }
            else {
                free_block = max_node->next;
            }
        }
        else {
            new_tail->next = max_node;
            new_tail = max_node;
            if (pre_max_node != NULL) {
                pre_max_node->next = max_node->next;
            }
            else {
                free_block = max_node->next;
            }
        }
    }
    new_tail->next = NULL;
    free_block = new_head;
}
/*创建新的进程，主要是获取内存的申请数量*/
int new_process()
{
    struct allocated_block* ab;
    int size;
    int ret;
    ab = (struct allocated_block*)malloc(sizeof(struct allocated_block));
    if (!ab)
        exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);//发送格式化输出到ab->process_name所指向的字符串
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if (size > 0)
        ab->size = size;
    ret = allocate_mem(ab); /* 从空闲区分配内存，ret==1 表示分配 ok*/
    /*如果此时 allocated_block_head 尚未赋值，则赋值*/
    if ((ret == 1) && (allocated_block_head == NULL))
    {
        allocated_block_head = ab;
        return 1;
    }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret == 1)
    {
        ab->next = allocated_block_head;
        allocated_block_head = ab;
        return 2;
    }
    else if (ret == -1)
    { /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return -1;
    }
    return 3;
}
/*分配内存模块*/
int allocate_mem(struct allocated_block* ab)
{
    //printf("allocate_mem");
    struct free_block_type* fbt, * pre;
    int request_size = ab->size;
    fbt = free_block;
    pre = free_block;
    while (fbt != NULL) {
        if (fbt->size <= request_size) {
            pre = fbt;
            fbt = fbt->next;
            continue;
        }
        if (fbt->size <= request_size + MIN_SLICE) {//找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
            if (fbt == pre) {
                free_block = free_block->next;
                ab->size = fbt->size;
                ab->start_addr = fbt->start_addr;
                free(fbt);
                rearrange(ma_algorithm);
                return 1;
            }
            else {
                pre->next = fbt->next;
                ab->size = fbt->size;
                ab->start_addr = fbt->start_addr;
                free(fbt);
                rearrange(ma_algorithm);
                return 1;
            }
        }
        else {//找到可满足空闲分区且分配后剩余空间足够大，则分割
            ab->start_addr = fbt->start_addr;
            fbt->start_addr = ab->start_addr + ab->size;
            fbt->size = fbt->size - ab->size;
            rearrange(ma_algorithm);
            return 1;
        }
    }
    //找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
    fbt = free_block;
    pre = free_block;
    fbt->start_addr = DEFAULT_MEM_START;
    int address = fbt->start_addr + fbt->size;
    while (fbt->next != NULL) {
        pre = fbt;
        fbt = fbt->next;
        fbt->start_addr = address;
        address += fbt->size;
    }
    struct allocated_block* _ab = allocated_block_head;
    struct allocated_block* _pre = allocated_block_head;
    _ab->start_addr = address;
    address += _ab->size;
    while (_ab->next != NULL) {
        _pre = _ab;
        _ab = _ab->next;
        _ab->start_addr = address;
        address += _ab->size;
    }
    //内存紧缩后再次尝试分配
    rearrange(ma_algorithm);
    request_size = ab->size;
    fbt = free_block;
    pre = free_block;
    while (fbt != NULL) {
        if (fbt->size <= request_size) {
            pre = fbt;
            fbt = fbt->next;
            continue;
        }
        if (fbt->size <= request_size + MIN_SLICE) {//找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
            if (fbt == pre) {
                free_block = free_block->next;
                ab->size = fbt->size;
                ab->start_addr = fbt->start_addr;
                free(fbt);
                rearrange(ma_algorithm);
                return 1;
            }
            else {
                pre->next = fbt->next;
                ab->size = fbt->size;
                ab->start_addr = fbt->start_addr;
                free(fbt);
                rearrange(ma_algorithm);
                return 1;
            }
        }
        else {//找到可满足空闲分区且分配后剩余空间足够大，则分割
            ab->start_addr = fbt->start_addr;
            fbt->start_addr = ab->start_addr + ab->size;
            fbt->size = fbt->size - ab->size;
            rearrange(ma_algorithm);
            return 1;
        }
    }
    //内存分配失败
    return -1;

    // 根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
    //  1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
    //  2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
    //  3. 找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
    // 4. 在成功分配内存后，应保持空闲分区按照相应算法有序
    // 5. 分配成功则返回 1，否则返回-1
    // 请自行补充。。。。。
}
/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
void kill_process()
{
    struct allocated_block* ab;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab = find_process(pid);
    if (ab != NULL)
    {
        free_mem(ab); /*释放 ab 所表示的分配区*/
        dispose(ab);  /*释放 ab 数据结构节点*/
    }
}
/*将 ab 所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block* ab)
{
    int algorithm = ma_algorithm;
    struct free_block_type* fbt, * prenode, * node;
    fbt = (struct free_block_type*)malloc(sizeof(struct free_block_type));
    if (!fbt)
        return -1;
    fbt->start_addr = ab->start_addr;
    fbt->size = ab->size;
    fbt->next = NULL;
    //将新节点插入到空闲分区队列末尾
    node = free_block;
    int num = 1;
    while (node->next != NULL) {
        num++;
        node = node->next;
    }
    node->next = fbt;
    num++;
    //对空闲链表按照起始地址有序排列
    node = free_block;
    struct free_block_type* new_head = NULL;
    struct free_block_type* new_tail = NULL;
    struct free_block_type* min_node;
    struct free_block_type* pre_min_node;
    for (int i = 0; i < num; i++) {
        min_node = free_block;
        pre_min_node = NULL;
        node = free_block;
        prenode = NULL;
        while (node != NULL) {
            if (node->start_addr < min_node->start_addr) {
                min_node = node;
                pre_min_node = prenode;
            }
            prenode = node;
            node = node->next;
        }
        //将每次循环中的最小空闲内存连接到新的链表中
        if (new_head == NULL) {
            new_head = min_node;
            new_tail = min_node;
            if (pre_min_node != NULL) {
                pre_min_node->next = min_node->next;
            }
            else {
                free_block = min_node->next;
            }
        }
        else {
            new_tail->next = min_node;
            new_tail = min_node;
            if (pre_min_node != NULL) {
                pre_min_node->next = min_node->next;
            }
            else {
                free_block = min_node->next;
            }
        }
    }
    new_tail->next = NULL;
    free_block = new_head;
    //检查并合并相邻的空闲分区,将空闲链表重新按照当前算法排序
    rearrange(algorithm);

    // 进行可能的合并，基本策略如下
    // 1. 将新释放的结点插入到空闲分区队列末尾
    // 2. 对空闲链表按照地址有序排列
    // 3. 检查并合并相邻的空闲分区
    // 4. 将空闲链表重新按照当前算法排序

    return 1;
}
/*释放 ab 数据结构节点*/
int dispose(struct allocated_block* free_ab)
{
    struct allocated_block* pre, * ab;
    if (free_ab == allocated_block_head)
    { /*如果要释放第一个节点*/
        allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while (ab != free_ab)
    {
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage()
{
    struct free_block_type* fbt = free_block;
    struct allocated_block* ab = allocated_block_head;
    if (fbt == NULL)
        return (-1);
    printf("----------------------------------------------------------\n"); /* 显示空闲区 */
    printf("Free Memory:\n");
    printf("%20s %20s\n", " start_addr", " size");
    while (fbt != NULL)
    {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }
    /* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while (ab != NULL)
    {
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name,
            ab->start_addr, ab->size);
        ab = ab->next;
    }
    printf("----------------------------------------------------------\n");
    return 0;
}
/*释放链表并退出 */
void do_exit() {
    struct free_block_type* fbt = free_block;
    struct free_block_type* node;
    while (fbt != NULL) {
        node = fbt;
        fbt = fbt->next;
        free(node);
    }
    struct allocated_block* ab = allocated_block_head;
    struct allocated_block* _node;
    while (ab != NULL) {
        _node = ab;
        ab = ab->next;
        free(_node);
    }
}
struct allocated_block* find_process(int pid) {
    struct allocated_block* ab = allocated_block_head;
    while (ab != NULL) {
        if (ab->pid == pid) {
            return ab;
        }
        ab = ab->next;
    }
    return NULL;
}
