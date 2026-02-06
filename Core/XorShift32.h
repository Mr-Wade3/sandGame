#pragma once
#include <cstdint>

struct XorShift32 {
    using result_type = uint32_t;

    uint32_t state = 0x12345678;

    static constexpr result_type min() {
        return 0;
    }

    static constexpr result_type max() {
        return UINT32_MAX;
    }

    result_type operator()() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
};