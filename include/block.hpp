#pragma once
#include <cstddef>

typedef struct mem_block{
    bool free;
    size_t size;
    bool is_aligned;
    struct mem_block* next = nullptr;
}mem_block_t;

#define MEM_BLOCK_SIZE sizeof(mem_block_t)