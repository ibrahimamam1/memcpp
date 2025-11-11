#include "include/alloc.hpp"
#include "include/block.hpp"
#include <cstddef>
#include <unistd.h>
#include <memory>
#include <mutex>

#define INITIAL_BLOCK_SIZE  1024
#define STARTING_SIZE INITIAL_BLOCK_SIZE+MEM_BLOCK_SIZE

mem_block_t* head = nullptr;
std::mutex alloc_mutex;

void init_mem_pool() {
    head = (mem_block_t*) sbrk(STARTING_SIZE);
    if(head == (void*) -1) {
        head = nullptr; // sbrk failed
        return;
    }
    head->size = INITIAL_BLOCK_SIZE;
    head->free = true;
    head->is_aligned = false;
    head->next = nullptr;
}

void* mem_alloc_align(size_t size, Alignment alignment = Alignment::ALIGN_NATURAL){
    size_t aligned_size = align_size(size, alignment) + sizeof(void*);
    
    void* unaligned = mem_alloc(aligned_size);
    if(unaligned == nullptr) return nullptr;

    //calculate aligned address
    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(unaligned);
    uintptr_t aligned_addr = (raw_addr + sizeof(void*) + aligned_size -1) & ~(aligned_size - 1);

    void** block_ptr_location = reinterpret_cast<void**>(aligned_addr - sizeof(void*));
    *block_ptr_location = unaligned;

    return reinterpret_cast<void*>(aligned_addr);
}

void* mem_alloc_align_type(size_t size, AlignmentForType type_alignment){
    return mem_alloc_align(size, static_cast<Alignment>(type_alignment));
}

void* mem_alloc(size_t size){
    std::lock_guard<std::mutex> lock(alloc_mutex);
    
    if(head == nullptr) {
        init_mem_pool();
    }
    mem_block_t* current = head;
    mem_block_t* prev = nullptr;
    while(current != nullptr){
       if(current->free && current->size == size){
            //Found a suitable block
            current->free = false;
            return (void*)(current + 1);
       }else if(current->free && current->size > size){
            size_t remaining_size = current->size - size;
            if(remaining_size <= MEM_BLOCK_SIZE){
                //Not enough space to split, allocate entire block
                current->free = false;
                return (void*)(current + 1);
            }

            //Large enough to split, split it
            mem_block_t* new_block = (mem_block_t*)((char*)(current + 1) + size);
            new_block->free = true;
            new_block->size = current->size - size - MEM_BLOCK_SIZE;
            new_block->is_aligned = false;
            new_block->next = current->next;

            //update current block
            current->free = false;
            current->size = size;
            current->is_aligned = false;
            current->next = new_block;
            return (void*)(current + 1); 
       }
       prev = current; 
       current = current->next;
    };

    //No suitable block found, request more memory
    mem_block_t* new_block = (mem_block_t*) sbrk(size + MEM_BLOCK_SIZE);
    if(new_block == (void*) -1) {
        return nullptr; //sbrk failed
    }
    new_block->size = size;
    new_block->free = false;
    new_block->is_aligned = false;
    new_block->next = nullptr;
    
    //Link the new block
    if (prev != nullptr) {
        prev->next = new_block;
    } else {
        head = new_block;  // This would be the first block
    }
    return (void*)(new_block + 1);
}

void mem_free(void* ptr) {
    std::lock_guard<std::mutex> lock(alloc_mutex);
    if(ptr == nullptr) return;
    
    mem_block_t* block = (mem_block_t*)ptr - 1;
    void* actual_ptr = ptr;
    
    if(block->is_aligned) {
        void** back_ptr = reinterpret_cast<void**>(ptr) - 1;
        actual_ptr = *back_ptr;
        block = (mem_block_t*)actual_ptr - 1;
    }
    block->free = true;

    //Coalesce adjacent free blocks
    if(block->next != nullptr && block->next->free) {
        block->size += MEM_BLOCK_SIZE + block->next->size;
        block->next = block->next->next;
    }
    mem_block_t* current = head;
    while(current->next != nullptr) {
        if(current->next == block){
            if(current->free) {
                current->size += MEM_BLOCK_SIZE + block->size;
                current->next = block->next;
            }
            break;
        }
        current = current->next;
    }
}