#ifndef HEAP_H
#define HEAP_H
#include <unistd.h>

typedef struct block_t
{
    int size;
    int free;
    struct block_t* next;
    struct block_t* prev;

} block_t;

typedef struct region_t
{
    struct region_t* next;
    int size;
    block_t* head;
} region_t;

region_t* create_region(size_t size);
void* _alloc(size_t size);
void  _free(void* ptr);

#endif