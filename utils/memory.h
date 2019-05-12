#ifndef MEMORY
#define MEMORY

#ifndef platform_big_alloc
#include <stdlib.h>
#define platform_big_alloc(size) malloc(size)
#endif

#include <utils/logging.h>
#include <utils/misc.h>

#ifndef N_MEMORY_STACKS
#define N_MEMORY_STACKS 4
#endif

byte* memory_stack_memory[N_MEMORY_STACKS];
size_t memory_stack_size[N_MEMORY_STACKS];
byte* memory_stack_start[N_MEMORY_STACKS];
byte* memory_stack_end[N_MEMORY_STACKS];
bool memory_stack_available[N_MEMORY_STACKS];

int init_memory()
{

    for(int i = 0; i < N_MEMORY_STACKS; i++)
    {
        memory_stack_size[i] = 500*megabyte;
        memory_stack_memory[i] = (byte*) platform_big_alloc(memory_stack_size[i]);
        memory_stack_start[i] = memory_stack_memory[i];
        memory_stack_end[i] = memory_stack_memory[i]+memory_stack_size[i];
        memory_stack_available[i] = true;
        assert(memory_stack_memory[i], "could not allocate free memory pool #", i);
    }
    return 0;
}
static int _ = init_memory();

//TODO: should add some randomization so that no single stack gets used a lot more

void* permalloc(size_t size)
{
    for(int i = 0; i < N_MEMORY_STACKS; i++)
    {
        if(memory_stack_available[i] && memory_stack_end[i]-memory_stack_memory[i] > size)
        {
            void* out = memory_stack_memory[i];
            memory_stack_memory[i] += size;
            return out;
        }
    }
    //TODO: should probably allocate more here or something
    assert("no free stacks available for permanent allocation of ", size, " bytes");
    return 0;
}

int reserve_stack()
{
    for(int i = 0; i < N_MEMORY_STACKS; i++)
    {
        if(memory_stack_available[i])
        {
            memory_stack_available[i] = false;
            return i;
        }
    }
    //TODO: should probably allocate more here or something
    assert("no free stacks available for reservation");
    return -1;
}

void unreserve_stack(int i)
{
    memory_stack_available[i] = true;
}

size_t available_free_memory(int i)
{
    return memory_stack_end[i]-memory_stack_memory[i];
}

#endif //MEMORY
