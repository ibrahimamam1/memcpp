#include "include/alloc.hpp"
#include "include/block.hpp"
#include <cstddef>
#include <unistd.h>
#include <memory>

#define INITIAL_BLOCK_SIZE  1024
#define STARTING_SIZE INITIAL_BLOCK_SIZE+MEM_BLOCK_SIZE

mem_block_t* head = nullptr;

void init_mem_pool() {
    head = (mem_block_t*) sbrk(STARTING_SIZE);
    head->size = INITIAL_BLOCK_SIZE;
    head->free = true;
    head->next = nullptr;
}

void* mem_alloc(size_t size) {
    if(head == nullptr) {
        init_mem_pool();
    }
    mem_block_t* current = head;
    do{
       if(current->free && current->size == size){
            //Found a suitable block
            current->free = false;
            return (void*)(current + MEM_BLOCK_SIZE);
       }else if(current->free && current->size > size){
            //Found a larger block, split it
            mem_block_t* new_block = (mem_block_t*)((char*)(current + MEM_BLOCK_SIZE) + size);
            new_block->free = true;
            new_block->size = current->size - size - MEM_BLOCK_SIZE;
            new_block->next = current->next;

            //update current block
            current->free = false;
            current->size = size;
            current->next = new_block;
            return (void*)(current + MEM_BLOCK_SIZE); 
       }
       current = current->next; 
    }while(current->next != nullptr);

    //No suitable block found, request more memory
    mem_block_t* new_block = (mem_block_t*) sbrk(size + MEM_BLOCK_SIZE);
    new_block->size = size;
    new_block->free = false;
    new_block->next = nullptr;
    
    //Link the new block
    current->next = new_block;
    return (void*)(new_block + MEM_BLOCK_SIZE);
}

void mem_free(void* ptr) {
    if(ptr == nullptr) return;
    mem_block_t* block = (mem_block_t*)((char*)ptr - MEM_BLOCK_SIZE);
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