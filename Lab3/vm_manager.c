#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define OFFSET_BITS 8//偏移地址位数
#define PAGE_BITS 8//虚拟页号位数
#define FRAME_BITS 7//物理页号位数  

#define PAGE_SIZE (1 << OFFSET_BITS)
#define FRAME_SIZE PAGE_SIZE
#define TLB_SIZE 16

#define NUMBER_OF_PAGES (1 << PAGE_BITS)
#define NUMBER_OF_FRAMES (1 << FRAME_BITS)
#define MEMORY_SIZE (FRAME_SIZE * NUMBER_OF_FRAMES)

#define BACKING_STORAGE_FILE "BACKING_STORE.bin"//模拟磁盘

typedef struct {
    uint8_t valid;
    uint32_t page, frame;
} T;

T TLB[TLB_SIZE];//模拟TLB

int8_t memory[MEMORY_SIZE];//模拟内存
uint32_t page_table[NUMBER_OF_PAGES];//模拟页表
uint32_t next_available_frame, next_available_TLB;
uint8_t page_valid[NUMBER_OF_PAGES];//模拟页表有效位
FILE *backing_storage, *input_file;

uint32_t total_cnt, page_fault_cnt, TLB_miss_cnt;

uint32_t get_page_number(uint32_t address) {
    uint32_t mask = (1 << (OFFSET_BITS + PAGE_BITS)) - (1 << OFFSET_BITS);
    uint32_t masked_address = address & mask;
    uint32_t page_number = masked_address >> OFFSET_BITS;
    return page_number;
}

uint32_t get_offset(uint32_t address) {
    uint32_t mask = (1 << OFFSET_BITS) - 1;
    uint32_t offset = address & mask;
    return offset;
}

uint32_t get_physical_address(uint32_t frame_number, uint32_t offset) {
    uint32_t physical_address = (frame_number << OFFSET_BITS) | offset;
    return physical_address;
}

uint32_t select_victim_frame() {//使用FIFO算法选取替换页面
    if(next_available_frame < NUMBER_OF_FRAMES) {
        return next_available_frame++;
    }
    uint32_t victim = (next_available_frame++) % NUMBER_OF_FRAMES;
    for(size_t i = 0; i < NUMBER_OF_PAGES; ++i) {
        if(page_valid[i] && page_table[i] == victim) {
            page_valid[i] = 0;
            break;
        }
    }
    return victim;
}

void handle_page_fault(uint32_t page_number) {//处理缺页错误
    page_table[page_number] = select_victim_frame();
    fseek(backing_storage, page_number * PAGE_SIZE, SEEK_SET);//移动文件读写指针
    fread(memory + page_table[page_number] * PAGE_SIZE, sizeof(int8_t), PAGE_SIZE, backing_storage);//实现从模拟磁盘将页帧换入模拟内存
    page_valid[page_number] = 1;
    ++page_fault_cnt;
}

int check_TLB(uint32_t page_number, uint32_t *frame_number) {//检查page_number是否在TLB中
    for(size_t i = 0; i < TLB_SIZE; ++i) {
        if(TLB[i].valid && TLB[i].page == page_number) {
            *frame_number = TLB[i].frame;
            return 1;
        }
    }
    return 0;
}

void update_TLB(uint32_t page_number, uint32_t frame_number) {//使用FIFO算法更新TLB
    size_t victim = next_available_TLB % TLB_SIZE;
    next_available_TLB = (next_available_TLB + 1) % TLB_SIZE;
    TLB[victim].valid = 1;
    TLB[victim].page = page_number;
    TLB[victim].frame = frame_number;
}


uint32_t translate_address(uint32_t logical) {//地址转化
    ++total_cnt;
    uint32_t page_number, offset, frame_number;
    page_number = get_page_number(logical);
    offset = get_offset(logical);
    if(!check_TLB(page_number, &frame_number)) {//TLB 未击中
        ++TLB_miss_cnt;
        if(page_valid[page_number] == 0) {//缺页处理
            handle_page_fault(page_number);
        }
        frame_number = page_table[page_number];
        update_TLB(page_number, frame_number);
    }
    return get_physical_address(frame_number, offset);
}

char access_memory(uint32_t physical) {
    return memory[physical];
}

int init(int argc, char **argv) {//算法初始化
    if(argc != 2) {//参数个数传递出错
        printf("Incorrect number of arguments.\n");
        return -1;
    }
    backing_storage = fopen(BACKING_STORAGE_FILE, "rb");
    if(backing_storage == NULL) {//文件打开失败
        printf("Unable to open the backing storage file: %s\n", BACKING_STORAGE_FILE);
        return -2;
    }
    input_file = fopen(argv[1], "r");
    if(input_file == NULL) {//文件打开失败
        printf("Unable to open the input file: %s\n", argv[1]);
        return -3;
    }
    memset(page_valid, 0, sizeof(uint8_t) * NUMBER_OF_PAGES);
    next_available_frame = next_available_TLB = 0;
    return 0;
}

void clean_up() {//关闭文件
    if(input_file) {
        fclose(input_file);
    }
    if(backing_storage) {
        fclose(backing_storage);
    }
}

void display_statistics() {
    printf("Page fault rate: %.2f%%\n", (float)page_fault_cnt / total_cnt * 100);
    printf("TLB hit rate: %.2f%%\n", (float)(total_cnt - TLB_miss_cnt) / total_cnt * 100);
}


int main(int argc, char **argv) {
    if(init(argc, argv) != 0) {
        clean_up();
        return 0;
    }
    char line[8];
    while(fgets(line, 8, input_file)) {//逐行读取input_file
        uint32_t logical, physical;
        int8_t value;
        sscanf(line, "%u", &logical);//将line解析为无符号整数并存入logical中
        physical = translate_address(logical);
        value = access_memory(physical);
        printf("Virtual address: %u Physical address: %u Value: %d\n", logical, physical, value);
    }
    display_statistics();
    clean_up();
    return 0;
}
