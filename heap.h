#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

// --------------------------------------
// Alignment for blocks (8 bytes)
// --------------------------------------
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// --------------------------------------
// Block structure
// --------------------------------------
typedef struct block {
    size_t size;          // Size of the block's data
    int free;             // 1 if free, 0 if allocated
    struct block* prev;   // Previous block in the region
    struct block* next;   // Next block in the region
} block_t;

// --------------------------------------
// Memory region structure
// --------------------------------------
typedef struct region {
    size_t size;          // Total size of the region
    block_t* head;        // Head of the block list
    struct region* next;  // Next region in the heap
} region_t;

// --------------------------------------
// Global heap regions pointer
// --------------------------------------
extern region_t* regions;

// --------------------------------------
// Allocator functions
// --------------------------------------
void* _alloc(size_t size);      // Allocate memory
void _free(void* ptr);          // Free memory

// --------------------------------------
// Helper functions (optional, can be used internally)
// --------------------------------------
region_t* create_region(size_t size);      // Create a new memory region
block_t* find_free_block(region_t* region, size_t size); // Find free block in a region
void split_block(block_t* block, size_t size);           // Split a block
void coalesce(block_t* block);                            // Merge adjacent free blocks

#endif // HEAP_H
