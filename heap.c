#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE (1024 * 1024)
#define ALIGNMENT 8  // 8-byte alignment for safety

// Round up to nearest multiple of ALIGNMENT
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

region_t* regions = NULL;

// --------------------------------------
// Create a new memory region
// --------------------------------------
region_t* create_region(size_t size)
{
    size = ALIGN(size);  // ensure region size is aligned
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return NULL;

    region_t* reg = (region_t*)mem;
    reg->next = NULL;
    reg->size = size;

    block_t* block = (block_t*)((char*)mem + sizeof(region_t));
    block->size = size - sizeof(region_t) - sizeof(block_t);
    block->free = 1;
    block->prev = NULL;
    block->next = NULL;

    reg->head = block;
    return reg;
}

// --------------------------------------
// Find a free block using first-fit
// --------------------------------------
block_t* find_free_block(region_t* region, size_t size)
{
    block_t* curr = region->head;
    while (curr)
    {
        if (curr->free && curr->size >= size)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

// --------------------------------------
// Split a block if large enough
// --------------------------------------
void split_block(block_t* block, size_t size)
{
    size = ALIGN(size);
    if (block->size < size + sizeof(block_t) + ALIGNMENT)
        return;  // not enough space to split

    block_t* remainder = (block_t*)((char*)block + sizeof(block_t) + size);
    remainder->size = block->size - size - sizeof(block_t);
    remainder->free = 1;
    remainder->next = block->next;
    remainder->prev = block;

    if (block->next)
        block->next->prev = remainder;

    block->next = remainder;
    block->size = size;
}

// --------------------------------------
// Allocate memory
// --------------------------------------
void* _alloc(size_t size)
{
    if (size == 0) return NULL;
    size = ALIGN(size);

    region_t* r = regions;
    block_t* block = NULL;

    while (r)
    {
        block = find_free_block(r, size);
        if (block) break;
        r = r->next;
    }

    if (!block)
    {
        region_t* new_region = create_region(HEAP_SIZE);
        if (!new_region) return NULL;

        new_region->next = regions;
        regions = new_region;
        block = new_region->head;
    }

    split_block(block, size);
    block->free = 0;
    return (char*)block + sizeof(block_t);
}

// --------------------------------------
// Coalesce adjacent free blocks
// --------------------------------------
void coalesce(block_t* block)
{
    // Merge with next
    if (block->next && block->next->free)
    {
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }

    // Merge with prev
    if (block->prev && block->prev->free)
    {
        block->prev->size += sizeof(block_t) + block->size;
        block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
        block = block->prev;
    }
}

// --------------------------------------
// Free memory
// --------------------------------------
void _free(void* ptr)
{
    if (!ptr) return;

    block_t* block = (block_t*)((char*)ptr - sizeof(block_t));
    block->free = 1;

    coalesce(block);
}
