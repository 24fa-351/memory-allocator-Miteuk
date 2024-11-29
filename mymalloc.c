#include "mymalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BLOCK_META_SIZE sizeof(block_t)
#define ALIGN(size) (((size) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))
#define MAX_HEAP_SIZE 1024 // Adjust as necessary

typedef struct block {
    size_t size;   // Size of the block
    int is_free;   // Free status
    struct block* next; // Not used directly in heap-based management
} block_t;

// Min-Heap Data Structures
typedef struct heap_node {
    size_t size;   // Size of the block
    void* address; // Address of the block
} heap_node_t;

typedef struct min_heap {
    heap_node_t nodes[MAX_HEAP_SIZE];
    size_t count; // Number of elements in the heap
} min_heap_t;

static min_heap_t free_heap = { .count = 0 };
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Min-Heap Operations
void heap_insert(size_t size, void* address) {
    if (free_heap.count >= MAX_HEAP_SIZE) {
        fprintf(stderr, "Heap overflow!\n");
        return;
    }

    size_t i = free_heap.count++;
    free_heap.nodes[i].size = size;
    free_heap.nodes[i].address = address;

    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (free_heap.nodes[i].size >= free_heap.nodes[parent].size) break;

        heap_node_t temp = free_heap.nodes[i];
        free_heap.nodes[i] = free_heap.nodes[parent];
        free_heap.nodes[parent] = temp;
        i = parent;
    }
}

heap_node_t heap_extract_min() {
    if (free_heap.count == 0) {
        fprintf(stderr, "Heap underflow!\n");
        return (heap_node_t){0, NULL};
    }

    heap_node_t min = free_heap.nodes[0];
    free_heap.nodes[0] = free_heap.nodes[--free_heap.count];

    size_t i = 0;
    while (i < free_heap.count) {
        size_t left = 2 * i + 1, right = 2 * i + 2, smallest = i;

        if (left < free_heap.count && free_heap.nodes[left].size < free_heap.nodes[smallest].size)
            smallest = left;
        if (right < free_heap.count && free_heap.nodes[right].size < free_heap.nodes[smallest].size)
            smallest = right;

        if (smallest == i) break;

        heap_node_t temp = free_heap.nodes[i];
        free_heap.nodes[i] = free_heap.nodes[smallest];
        free_heap.nodes[smallest] = temp;
        i = smallest;
    }

    return min;
}

// Memory Manager Implementation
void* my_malloc(size_t size) {
    pthread_mutex_lock(&lock);

    size = ALIGN(size);
    heap_node_t block = heap_extract_min();
    if (block.address && block.size >= size) {
        block_t* blk = (block_t*)block.address;

        if (blk->size > size + BLOCK_META_SIZE) {
            // Split the block
            block_t* new_block = (block_t*)((char*)blk + BLOCK_META_SIZE + size);
            new_block->size = blk->size - size - BLOCK_META_SIZE;
            new_block->is_free = 1;
            heap_insert(new_block->size, new_block);
        }

        blk->is_free = 0;
        pthread_mutex_unlock(&lock);

        memset((char*)blk + BLOCK_META_SIZE, 0, size);
        return (char*)blk + BLOCK_META_SIZE;
    }

    // No suitable block found, extend heap
    void* heap_end = sbrk(size + BLOCK_META_SIZE);
    if (heap_end == (void*)-1) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    block_t* new_block = (block_t*)heap_end;
    new_block->size = size;
    new_block->is_free = 0;

    pthread_mutex_unlock(&lock);
    return (char*)new_block + BLOCK_META_SIZE;
}

void my_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&lock);

    block_t* block = (block_t*)((char*)ptr - BLOCK_META_SIZE);
    block->is_free = 1;

    heap_insert(block->size, block);
    pthread_mutex_unlock(&lock);
}

void* my_realloc(void* ptr, size_t size) {
    if (!ptr) return my_malloc(size);

    block_t* block = (block_t*)((char*)ptr - BLOCK_META_SIZE);
    if (block->size >= size) return ptr;

    void* new_ptr = my_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
    }
    return new_ptr;
}
