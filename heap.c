#include "heap.h"
#include <stdio.h>
#include <sys/mman.h>
#include <stddef.h>

#define HEAP_SIZE (1024*1024)



region_t* regions = NULL;

region_t* create_region(size_t size)
{
    void* mem = mmap(NULL,size, PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(mem == MAP_FAILED) return NULL;

    region_t* heap_region = (region_t*) mem;
    heap_region->next = NULL;
    heap_region->size = HEAP_SIZE;

    block_t* head_block = (block_t*)((char*) mem + sizeof(region_t));
    head_block->size = HEAP_SIZE;
    head_block->free = 1;
    head_block->prev = NULL;
    head_block->next = NULL;

    heap_region->head = head_block;
    return heap_region;
}

block_t* find_free_region(region_t* region, size_t size)
{
    block_t* curr = region->head;

    while(curr)
    {
        if(curr->free && curr->size >= size) return curr;
        curr = curr->next;
    }

    return NULL;
}

void* _alloc(size_t size)
{
    region_t* r = regions;
    block_t* wanted_block = NULL;

    while(r)
    {
        wanted_block = find_free_region(r,size);
        if(wanted_block) break;
        r = r->next;
    }

    if(!wanted_block)
    {
        region_t* new_region = create_region(HEAP_SIZE);
        if(!new_region) return NULL;

        new_region->next = regions;
        regions = new_region;

        wanted_block = new_region->head;
    }


    if(wanted_block->free && wanted_block->size > size && wanted_block->size - size > sizeof(block_t*))
    {
        char* pointer = ((char*)wanted_block + sizeof(block_t) + size);

        block_t* remainder = (block_t*) pointer;
        remainder->free = 1;
        remainder->next = wanted_block->next;
        remainder->prev = wanted_block;
        remainder->size = wanted_block->size - size - sizeof(block_t);

        if(wanted_block->next) wanted_block->next->prev = remainder;
        wanted_block->next = remainder;
        wanted_block->size = size;
        
    }

    wanted_block->free = 0;
    return (char*) wanted_block + sizeof(block_t);

}

void _free(void* ptr)
{
    if(!ptr) return;

    block_t* our_block = (block_t*)((char*) ptr - sizeof(block_t));
    our_block->free = 1;

    block_t* curr = our_block;

    while (curr && curr->next)
    {
        if(curr->free && curr->next->free)
        {
            curr->size += sizeof(block_t) + curr->next->size;
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
    

}