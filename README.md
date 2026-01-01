# memcpp
lightweight c++ memory management library.

## Features
- Manages process internal address space.
- Byte Alignment.
- Large Allocations Support.
- Coalescense of freed memory.
- Lightweight and fast.
- Intuitive API.

## How To Install
### Prerequisites
- cmake
- ninja

```bash
git clone https://github.com/ibrahimamam1/memcpp
cd memcpp
cmake .
cd build/
ninja install
```

## How To Use
```c 
#include<alloc.hpp>
#include<alignment.hpp>

void* my_addr = mem_alloc(64); //reserves 64 bytes memory on the heap and returns starting address. memory is unaligned.

void* my_addr_aligned = mem_alloc_aligned(64, Alignment::Align16); //memory is 16 bit aligned

//free memory
mem_free(my_addr)
mem_free(my_addr_aligned)
``` 

### To compile
```bash
g++ my_file.cpp -lmemcpp
```


