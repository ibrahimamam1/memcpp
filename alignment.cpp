#include "include/alignment.hpp"


size_t align_size(size_t size, Alignment alignment){
    size_t align = static_cast<size_t>(alignment);
    if(size % align == 0){
        return size;
    }
    return size + (align - (size % align)); 
}
size_t align_size(size_t size, AlignmentForType type_alignment){
    return align_size(size, static_cast<Alignment>(type_alignment));
}