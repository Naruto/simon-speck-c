//
// Created by naruto on 16/08/11.
//

#ifndef SIMON_SPECK_SPECK_H
#define SIMON_SPECK_SPECK_H

#include <cstdint>

using s64 = int_fast64_t;
using u64 = uint_fast64_t;
using u32 = uint_fast32_t;
using u16 = uint_fast16_t;
using u8  = uint_fast8_t;

// number of round
enum class spec {
    ROUND_32_64,
    ROUND_48_72,
    ROUND_48_96,
    ROUND_64_96,
    ROUND_64_128,
    ROUND_96_96,
    ROUND_96_144,
    ROUND_128_128,
    ROUND_128_192,
    ROUND_128_256,
};

//left circular shift
#define LCS _lrotl
//right circular shift
#define RCS _lrotr


extern "C" {
     bool Speck_init(enum spec idx);

};
#endif //SIMON_SPECK_SPECK_H
