#include <cstddef>
#pragma once

enum Alignment : size_t {
    ALIGN_1   = 1,
    ALIGN_2   = 2,
    ALIGN_4   = 4,
    ALIGN_8   = 8,
    ALIGN_16  = 16,
    ALIGN_32  = 32,
    ALIGN_64  = 64,
    ALIGN_128 = 128,
    ALIGN_256 = 256,
    ALIGN_512 = 512,
    ALIGN_1024= 1024,
    ALIGN_NATURAL = alignof(std::max_align_t)
};

enum AlignmentForType {
    ALIGN_CHAR        = ALIGN_1,
    ALIGN_SHORT       = ALIGN_2,
    ALIGN_INT         = ALIGN_4,
    ALIGN_LONG        = ALIGN_8,
    ALIGN_LONG_LONG   = ALIGN_8,
    ALIGN_FLOAT       = ALIGN_4,
    ALIGN_DOUBLE      = ALIGN_8,
    ALIGN_LONG_DOUBLE = ALIGN_16,
    ALIGN_POINTER     = ALIGN_8,
    ALIGN_OBJECT      = ALIGN_NATURAL
};

size_t align_size(size_t size, Alignment alignment = ALIGN_NATURAL);
size_t align_size(size_t size, AlignmentForType type_alignment);