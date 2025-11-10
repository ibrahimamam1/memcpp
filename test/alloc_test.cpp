#include<gtest/gtest.h>
#include "../include/alloc.hpp"
#include <cstring>

TEST(AllocTest, SimpleAllocation) {
    size_t alloc_size = 128;
    void* ptr = mem_alloc(alloc_size);
    ASSERT_NE(ptr, nullptr) << "mem_alloc returned nullptr";
    
    // Check if the allocated memory is writable
    memset(ptr, 0, alloc_size);
    mem_free(ptr);
}