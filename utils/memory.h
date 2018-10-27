#ifndef MEMORY
#define MEMORY

#include <utils/misc.h>

byte* free_memory;
size_t free_memory_size;

void init_memory()
{
    free_memory_size = 1*gigabyte;
    free_memory = (byte*) platform_big_alloc(free_memory_size);
    assert(free_memory, "could not allocate free memory pool");
}

void* permalloc(uint size)
{
    void* out = free_memory;
    free_memory += size;
    return out;
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
