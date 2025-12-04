#ifndef MEMORY_H
#define MEMORY_H

#define MAX_BLOCKS 50
#define TOTAL_MEMORY 256   // you can change


#include "stdint.h"
#include "stddef.h"
#include "bool.h"
#include "../include/tty.h"

#define EINVARG 2
#define ENOMEM 3

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000
#define HEAP_ADDRESS 0x01000000
#define HEAP_TABLE_ADRESS 0x00007E00

// 100 MB heap size
#define HEAP_SIZE_BYTES 1024 * 1024 * 100
// 4 kb block size
#define HEAP_BLOCK_SIZE 4096

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY *entries;
    size_t total;
};

struct heap
{
    struct heap_table *table;
    // start address of the heap data pool
    void *saddr;
};

// static struct heap kernel_heap;
// static struct heap_table kernel_heap_table;
typedef struct {
    int pid;
    int start;
    int size;
    int free;  // 1 = free, 0 = allocated
} Block;


void memory_menu();
void memory_init();

void heap_init();
int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table);
void memcpy(void *dest, void *src, size_t n);
void *heap_malloc(struct heap *heap, size_t size);
void heap_free(struct heap *heap, void *ptr);
int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry);
void *kmalloc(size_t size);
void kfree(void *ptr);

// Educational memory management functions
void memory_stats(void);
void memory_map(void);
void memory_demo(void);
void memory_test(void);
size_t get_used_memory(void);
size_t get_free_memory(void);
size_t get_total_blocks(void);
size_t get_used_blocks(void);
float get_fragmentation_ratio(void);

#endif

// #ifndef MEMORY_H
// #define MEMORY_H

// #define MAX_BLOCKS 50
// #define TOTAL_MEMORY 256   // you can change

// typedef struct {
//     int pid;
//     int start;
//     int size;
//     int free;  // 1 = free, 0 = allocated
// } Block;

// void memory_menu();
// void memory_init();

// #endif
