#pragma once
#include <cstdint>

uint32_t rng = 0x12345678;

inline bool randBit() {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng & 1;
}