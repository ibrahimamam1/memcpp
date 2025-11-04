#include "include/alloc.hpp"
#include "include/block.hpp"
#include <cstddef>
#include <unistd.h>

#define INITIAL_BLOCKS  1024
#define STARTING_SIZE INITIAL_BLOCKS*BLOCK_SIZE

mem_block_t* head = nullptr;
void init_mem_pool() {
    head = (mem_block_t*)sbrk(STARTING_SIZE);
}

void* mem_alloc(size_t size) {
    if(head == nullptr) {
        init_mem_pool();
    }
    
    return nullptr;
}