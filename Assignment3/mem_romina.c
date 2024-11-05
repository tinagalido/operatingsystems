#include "mem.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

#define ALIGNMENT 8 // Align to 8 bytes
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define BLOCK_SIZE sizeof(mem_block)

typedef struct mem_block {
    size_t size;
    struct mem_block *next;
    int free;
} mem_block;

// Global variables
static mem_block *mem_start = NULL;
int m_error;

// Forward declarations for internal functions
static mem_block *find_fit(size_t size, int style);
static void split_block(mem_block *block, size_t size);
static void merge();

// mem_init implementation
int mem_init(int size_of_region) {
    if (mem_start != NULL || size_of_region <= 0) {
        m_error = E_BAD_ARGS;
        return -1;
    }
    int page_size = getpagesize();
    size_of_region = (size_of_region + page_size - 1) & ~(page_size - 1); // Align to page size
    mem_start = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem_start == MAP_FAILED) {
        m_error = E_BAD_ARGS;
        return -1;
    }
    mem_start->size = size_of_region - BLOCK_SIZE;
    mem_start->next = NULL;
    mem_start->free = 1;
    return 0;
}

// mem_alloc implementation
void *mem_alloc(int size, int style) {
    if (!size) return NULL;
    size = ALIGN(size + BLOCK_SIZE);
    mem_block *block = find_fit(size, style);
    if (block) {
        if ((block->size - size) >= (BLOCK_SIZE + ALIGNMENT)) {
            split_block(block, size);
        }
        block->free = 0;
        return (void*)(block + 1);
    }
    return NULL;
}

// mem_free implementation
int mem_free(void *ptr) {
    if (!ptr) return -1;
    mem_block *block = (mem_block*)ptr - 1;
    block->free = 1;
    merge();
    return 0;
}

// mem_dump implementation
void mem_dump() {
    mem_block *current = mem_start;
    while (current) {
        printf("Block at %p, size %lu, free %d\n", (void*)current, current->size, current->free);
        current = current->next;
    }
}

// Minimal implementations of internal functions to address linker errors
static mem_block *find_fit(size_t size, int style) {
    // Placeholder implementation - should be replaced with actual logic
    for (mem_block *block = mem_start; block != NULL; block = block->next) {
        if (block->free && block->size >= size) {
            return block;
        }
    }
    return NULL;
}

static void split_block(mem_block *block, size_t size) {
    // Placeholder implementation - should be replaced with actual logic
}

static void merge() {
    // Placeholder implementation - should be replaced with actual logic
}

// Simple main function for testing
int main() {
    if (mem_init(1024) != 0) {
        fprintf(stderr, "mem_init failed\n");
        return 1;
    }

    void *ptr1 = mem_alloc(128, M_FIRSTFIT);
    if (ptr1 == NULL) {
        fprintf(stderr, "mem_alloc failed\n");
        return 1;
    } else {
        printf("mem_alloc succeeded\n");
    }

    mem_dump();
    mem_free(ptr1);
    mem_dump();

    return 0;
}
