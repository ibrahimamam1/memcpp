# memcpp
small and lightweight c++ memory management library.

## How To Install
```bash
git clone https://github.com/ibrahimamam1/memcpp
cd memcpp
cmake .
cd build/
ninja install
```

## How To Use
#include<alloc.hpp>
#include<alignment.hpp>

void* my_addr = mem_alloc(64); //reserves 64 bytes memory on the heap and returns starting address. memory is unaligned.

void* my_addr_aligned = mem_alloc_aligned(64, Alignment::Align16); //memory is 16 bit aligned

//free memory
mem_free(my_addr)
mem_free(my_addr_aligned)

### To compile
```bash
g++ my_file.cpp -lmemcpp
```

## Features
- Manages process internal address space.
- Requested Memory can be Byte Aligned.
- Supports Large Allocations.
- Coalesces freed memory to reduce internal fragmentation.
- Lightweight and fast.
- Intuitive API.
