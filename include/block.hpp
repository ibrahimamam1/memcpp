#pragma once
#include <cstddef>

typedef struct mem_block{
    size_t size;
    bool free;
    struct mem_block* next;
    void* address;
}mem_block_t;

#define BLOCK_SIZE sizeof(mem_block_t)