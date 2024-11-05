/**
 * Submitted by: Maria Cristina Galido (2092015); 
 * Shabnam Alizadeh (2084527); 
 * Romina Sharifi (2055127)
 * 
 * The Makefile is supposed to work with mem.c, along with the header file in the code.
 * Run the following commands to compile and run the codes
 * make
 * make run
 * make clean
**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mem.h"

// Define the structure for a block of memory
typedef struct block
{
    size_t size;        // Size of the block
    int free;           // Flag to indicate if the block is free or not
    struct block *next; // Pointer to the next block
} Block;

Block *freeList = NULL; // The head of our linked list
int m_error;

// Function to split a block into two if the requested size is smaller than the block size
void split(Block *fitting_slot, size_t size)
{
    Block *new = (void *)((void *)fitting_slot + size + sizeof(Block));
    new->size = (fitting_slot->size) - size - sizeof(Block);
    new->free = 1;
    new->next = fitting_slot->next;
    fitting_slot->size = size;
    fitting_slot->free = 0;
    fitting_slot->next = new;
}

// Function to merge adjacent free blocks to avoid fragmentation
void merge()
{
    Block *curr = freeList;
    while ((curr != NULL) && (curr->next != NULL))
    {
        if ((curr->free) && (curr->next->free))
        {
            curr->size += (curr->next->size) + sizeof(Block);
            curr->next = curr->next->next;
        }
        curr = curr->next;
    }
}

// Function to initialize the memory allocator
int mem_init(int size_of_region)
{
    void *p = mmap(0, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == (void *)-1)
    {
        m_error = E_NO_SPACE;
        return -1;
    }
    freeList = p;
    freeList->size = size_of_region - sizeof(Block);
    freeList->free = 1;
    freeList->next = NULL;
    return 0;
}

// Function to allocate memory
void *mem_alloc(int size, int style)
{
    Block *curr = freeList;
    while ((((curr->size) < size) || ((curr->free) == 0)) && (curr->next != NULL))
    {
        curr = curr->next;
    }
    if ((curr->size) == size)
    {
        curr->free = 0;
    }
    else if ((curr->size) > (size + sizeof(Block)))
    {
        split(curr, size);
    }
    else
    {
        m_error = E_NO_SPACE;
        return NULL;
    }
    return (void *)(++curr);
}

// Function to free allocated memory
int mem_free(void *ptr)
{
    if (((void *)freeList <= ptr) && (ptr <= (void *)(freeList + sizeof(Block))))
    {
        Block *curr = ptr;
        --curr;
        curr->free = 1;
        merge();
    }
    else
    {
        m_error = E_BAD_POINTER;
        return -1;
    }
    return 0;
}

// Function to print the regions of free memory to the console
void mem_dump()
{
    Block *curr = freeList;
    printf("[Start]           [End]                 [Size]              [Free]\n");
    while (curr != NULL)
    {
        printf("[%p] - [%p] : [%zu] bytes, Free status: [%s]\n", 
            curr, 
            curr + sizeof(Block) + curr->size, 
            curr->size, 
            (curr->free) ? "Yes" : "No");
        curr = curr->next;
    }
}

// Main function for testing purposes
int main()
{

    if (mem_init(10000) != 0)
    {
        fprintf(stderr, "Memory initialization failed.\n");
        return 1;
    }

    void *p1 = mem_alloc(1000, M_FIRSTFIT);
    if (p1 == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    } else
    {
        printf("Memory allocation succeeded.\n");
    }

    void *p2 = mem_alloc(2000, M_FIRSTFIT);
    if (p2 == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        mem_free(p1);       // Free the previously allocated memory
        return 1;
    } else
    {
        printf("Memory allocation succeeded.\n");
    }

    mem_free(p1);

    // Print memory usage
    mem_dump();

    return 0;

}
