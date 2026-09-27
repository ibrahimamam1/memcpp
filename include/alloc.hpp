#pragma once
#include "alignment.hpp"

typedef struct Stats {
    size_t total_mem;
    size_t n_blocks_allocated;
    size_t mem_size_allocated;
    size_t num_blocks_free;
    size_t mem_size_free;
    size_t largest_free_block;
} Stats;

void* mem_alloc(size_t size);
void* mem_alloc_first_fit(size_t size);
void* mem_alloc_best_fit(size_t size);
void* mem_alloc_next_fit(size_t size);
void* mem_alloc_align(size_t size, Alignment alignment);
void* mem_alloc_align_type(size_t size, AlignmentForType type_alignment);
void mem_free(void* ptr);
Stats get_stats();