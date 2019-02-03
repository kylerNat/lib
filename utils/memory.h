#ifndef MEMORY
#define MEMORY

#ifndef platform_big_alloc
#include <stdlib.h>
#define platform_big_alloc(size) malloc(size)
#endif

#include <utils/misc.h>

byte* free_memory;
size_t free_memory_size;

byte* free_memory_start;
byte* free_memory_end;

int init_memory()
{
    free_memory_size = 1*gigabyte;
    free_memory = (byte*) platform_big_alloc(free_memory_size);
    free_memory_start = free_memory;
    free_memory_end = free_memory+free_memory_size;
    assert(free_memory, "could not allocate free memory pool");
    return 0;
}
static int _ = init_memory();

void* permalloc(size_t size)
{
    void* out = free_memory;
    free_memory += size;
    return out;
}

size_t available_free_memory()
{
    return free_memory-free_memory_start;
}

// #define MAX_MEMORY_BLOCKS 100
// #define MEMORY_BLOCK_SIZE 1024*1024*1024
// //linked list of large memory blocks, never get deallocated
// struct memory_block
// {
//     void* block_start;
//     void* block_end;
//     memory_block* prev;
//     memory_block* next;
// };

// memory_block current_block;
// void*

// void init_memory()
// {
//     current_block = (memory_block*) malloc(MAX_MEMORY_BLOCKS*sizeof(memory_block));
// }

// //allocates some memory which should be held for the life of the program
// byte* permalloc(int size)
// {
//     if
// }

#endif //MEMORY
