#include <gtest/gtest.h>
#include "../include/alloc.hpp"
#include <cstring>
#include <cstdint>
#include <vector>
#include <thread>

// Helper function to check if pointer is aligned
bool is_aligned(void* ptr, size_t alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

// ============================================================================
// Basic Allocation Tests
// ============================================================================

TEST(AllocTest, SimpleAllocation) {
    size_t alloc_size = 128;
    void* ptr = mem_alloc(alloc_size);
    
    ASSERT_NE(ptr, nullptr) << "mem_alloc returned nullptr";
    
    // Check if the allocated memory is writable
    memset(ptr, 0xAB, alloc_size);
    
    // Verify we can read back what we wrote
    unsigned char* byte_ptr = static_cast<unsigned char*>(ptr);
    for(size_t i = 0; i < alloc_size; i++) {
        EXPECT_EQ(byte_ptr[i], 0xAB) << "Memory corruption at byte " << i;
    }
    
    mem_free(ptr);
}

TEST(AllocTest, NullAllocation) {
    void* ptr = mem_alloc(0);
    // Just ensure it doesn't crash
    if(ptr != nullptr) {
        mem_free(ptr);
    }
}

TEST(AllocTest, MultipleAllocations) {
    const int num_allocs = 10;
    void* ptrs[num_allocs];
    
    // Allocate multiple blocks
    for(int i = 0; i < num_allocs; i++) {
        ptrs[i] = mem_alloc(64 + i * 16);
        ASSERT_NE(ptrs[i], nullptr) << "Allocation " << i << " failed";
        
        // Write unique pattern to each block
        memset(ptrs[i], i, 64 + i * 16);
    }
    
    // Verify each block still has correct data
    for(int i = 0; i < num_allocs; i++) {
        unsigned char* byte_ptr = static_cast<unsigned char*>(ptrs[i]);
        EXPECT_EQ(byte_ptr[0], static_cast<unsigned char>(i)) 
            << "Block " << i << " was corrupted";
    }
    
    // Free all blocks
    for(int i = 0; i < num_allocs; i++) {
        mem_free(ptrs[i]);
    }
}

TEST(AllocTest, LargeAllocation) {
    size_t large_size = 1024 * 1024; // 1 MB
    void* ptr = mem_alloc(large_size);
    
    ASSERT_NE(ptr, nullptr) << "Large allocation failed";
    
    // Write to first, middle, and last bytes
    unsigned char* byte_ptr = static_cast<unsigned char*>(ptr);
    byte_ptr[0] = 0xFF;
    byte_ptr[large_size / 2] = 0xAA;
    byte_ptr[large_size - 1] = 0x55;
    
    EXPECT_EQ(byte_ptr[0], 0xFF);
    EXPECT_EQ(byte_ptr[large_size / 2], 0xAA);
    EXPECT_EQ(byte_ptr[large_size - 1], 0x55);
    
    mem_free(ptr);
}

// ============================================================================
// Free and Reuse Tests
// ============================================================================

TEST(AllocTest, FreeAndReuse) {
    void* ptr1 = mem_alloc(128);
    ASSERT_NE(ptr1, nullptr);
    
    // Write pattern
    memset(ptr1, 0xDE, 128);
    
    mem_free(ptr1);
    
    // Allocate same size - might reuse the block
    void* ptr2 = mem_alloc(128);
    ASSERT_NE(ptr2, nullptr);
    
    // Should be able to write to it
    memset(ptr2, 0xAD, 128);
    
    mem_free(ptr2);
}

TEST(AllocTest, FragmentationAndCoalescing) {
    // Allocate 3 adjacent blocks
    void* ptr1 = mem_alloc(100);
    void* ptr2 = mem_alloc(100);
    void* ptr3 = mem_alloc(100);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    
    // Free middle block
    mem_free(ptr2);
    
    // Free first block - should coalesce with middle
    mem_free(ptr1);
    
    // Should be able to allocate larger block now
    void* ptr_large = mem_alloc(200);
    EXPECT_NE(ptr_large, nullptr) << "Coalescing may have failed";
    
    mem_free(ptr_large);
    mem_free(ptr3);
}

TEST(AllocTest, FreeNull) {
    // Freeing null should not crash
    EXPECT_NO_THROW(mem_free(nullptr));
}

TEST(AllocTest, DoubleFree) {
    void* ptr = mem_alloc(64);
    ASSERT_NE(ptr, nullptr);
    
    mem_free(ptr);
    
    // Double free - behavior is undefined, but shouldn't crash in your impl
    // since you just mark as free. In production, this should be detected.
    EXPECT_NO_THROW(mem_free(ptr));
}

// ============================================================================
// Aligned Allocation Tests
// ============================================================================

TEST(AllocTest, AlignedAllocation_16) {
    size_t alloc_size = 120;
    void* ptr = mem_alloc_align(alloc_size, Alignment::ALIGN_16);
    
    ASSERT_NE(ptr, nullptr) << "Aligned allocation failed";
    EXPECT_TRUE(is_aligned(ptr, 16)) 
        << "Pointer " << ptr << " is not 16-byte aligned";
    
    // Test writability
    memset(ptr, 0xFF, alloc_size);
    
    mem_free(ptr);
}

TEST(AllocTest, AlignedAllocation_128) {
    size_t alloc_size = 120;
    void* ptr = mem_alloc_align(alloc_size, Alignment::ALIGN_128);
    
    ASSERT_NE(ptr, nullptr) << "128-byte aligned allocation failed";
    EXPECT_TRUE(is_aligned(ptr, 128)) 
        << "Pointer " << ptr << " is not 128-byte aligned";
    
    memset(ptr, 0xCC, alloc_size);
    
    mem_free(ptr);
}

TEST(AllocTest, AlignedAllocation_Various) {
    struct TestCase {
        size_t size;
        Alignment alignment;
    };
    
    TestCase cases[] = {
        {32, ALIGN_8},
        {64, ALIGN_16},
        {100, ALIGN_32},
        {200, ALIGN_64},
        {500, ALIGN_256},
        {1000, ALIGN_512}
    };
    
    for(const auto& tc : cases) {
        void* ptr = mem_alloc_align(tc.size, tc.alignment);
        
        ASSERT_NE(ptr, nullptr) 
            << "Failed to allocate " << tc.size << " bytes with " 
            << tc.alignment << " alignment";
        
        EXPECT_TRUE(is_aligned(ptr, tc.alignment))
            << "Pointer not aligned to " << tc.alignment << " bytes";
        
        // Test memory is usable
        memset(ptr, 0x55, tc.size);
        
        mem_free(ptr);
    }
}

TEST(AllocTest, AlignedAllocation_TypeBased) {
    // Test type-based alignment
    //void* char_ptr = mem_alloc_align_type(100, ALIGN_CHAR);
    void* int_ptr = mem_alloc_align_type(100, ALIGN_INT);
    void* double_ptr = mem_alloc_align_type(100, ALIGN_DOUBLE);
    void* ptr_ptr = mem_alloc_align_type(100, ALIGN_POINTER);
    
    //ASSERT_NE(char_ptr, nullptr);
    ASSERT_NE(int_ptr, nullptr);
    ASSERT_NE(double_ptr, nullptr);
    ASSERT_NE(ptr_ptr, nullptr);
    
    EXPECT_TRUE(is_aligned(int_ptr, 4));
    EXPECT_TRUE(is_aligned(double_ptr, 8));
    EXPECT_TRUE(is_aligned(ptr_ptr, 8));
    
    //mem_free(char_ptr);
    mem_free(int_ptr);
    mem_free(double_ptr);
    mem_free(ptr_ptr);
}

TEST(AllocTest, AlignedArray) {
    // Simulate SIMD vector array
    struct alignas(16) Vec4 {
        float x, y, z, w;
    };
    
    const int count = 10;
    size_t aligned_size = align_size(sizeof(Vec4), ALIGN_16);
    void* ptr = mem_alloc_align(count * aligned_size, ALIGN_16);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(is_aligned(ptr, 16));
    
    // Cast to array and test each element alignment
    Vec4* array = static_cast<Vec4*>(ptr);
    for(int i = 0; i < count; i++) {
        EXPECT_TRUE(is_aligned(&array[i], 16)) 
            << "Array element " << i << " not aligned";
        
        // Test writability
        array[i].x = i * 1.0f;
        array[i].y = i * 2.0f;
        array[i].z = i * 3.0f;
        array[i].w = i * 4.0f;
    }
    
    // Verify values
    for(int i = 0; i < count; i++) {
        EXPECT_FLOAT_EQ(array[i].x, i * 1.0f);
        EXPECT_FLOAT_EQ(array[i].y, i * 2.0f);
    }
    
    mem_free(ptr);
}

// Thread Safety Tests
TEST(AllocTest, ThreadSafety_BasicConcurrency) {
    const int num_threads = 4;
    const int allocs_per_thread = 100;
    
    auto worker = [allocs_per_thread]() {
        std::vector<void*> local_ptrs;
        
        for(int i = 0; i < allocs_per_thread; i++) {
            void* ptr = mem_alloc(64);
            if(ptr != nullptr) {
                memset(ptr, 0xAA, 64);
                local_ptrs.push_back(ptr);
            }
        }
        
        for(void* ptr : local_ptrs) {
            mem_free(ptr);
        }
    };
    
    std::vector<std::thread> threads;
    for(int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker);
    }
    
    for(auto& t : threads) {
        t.join();
    }
    
    // If we get here without crashing, thread safety is likely working
    SUCCEED();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(AllocTest, AllocAfterManyFrees) {
    // Create fragmentation
    void* ptrs[10];
    for(int i = 0; i < 10; i++) {
        ptrs[i] = mem_alloc(100);
    }
    
    // Free every other block
    for(int i = 0; i < 10; i += 2) {
        mem_free(ptrs[i]);
    }
    
    // Try to allocate - should succeed
    void* new_ptr = mem_alloc(50);
    EXPECT_NE(new_ptr, nullptr);
    
    // Cleanup
    mem_free(new_ptr);
    for(int i = 1; i < 10; i += 2) {
        mem_free(ptrs[i]);
    }
}


TEST(AllocTest, VerySmallAllocation) {
    void* ptr = mem_alloc(1);
    ASSERT_NE(ptr, nullptr);
    
    unsigned char* byte = static_cast<unsigned char*>(ptr);
    *byte = 0x42;
    EXPECT_EQ(*byte, 0x42);
    
    mem_free(ptr);
}