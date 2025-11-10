#pragma once
#include "alignment.hpp"

void* mem_alloc(size_t size);
void* mem_alloc_align(size_t size, Alignment alignment);
void* mem_alloc_align_type(size_t size, AlignmentForType type_alignment);
void mem_free(void* ptr);