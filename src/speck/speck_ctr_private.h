/*
 * Copyright (c) 2016-2017 Naruto TAKAHASHI <tnaruto@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SPECK_CTR_H__
#define __SPECK_CTR_H__

#include <stdint.h>

/* increment counter (128-bit int) by 1 */
static inline uint8_t *ctr128_inc(uint8_t counter[16]) {
    uint32_t n = 16;
    uint8_t c;
    do {
        --n;
        c = counter[n];
        ++c;
        counter[n] = c;
        if (c) return counter;
    } while (n);

    return counter;
}

static void cast_uint8_array_to_uint64_len(uint64_t *dst, const uint8_t *array, int len) {
    uint8_t tmp[8] = {0};
    int i;
    for(i=0; i<len; i++) {
        tmp[i] = array[i];
    }
    *dst =  (uint64_t)tmp[7] << 56 | (uint64_t)tmp[6] << 48 | (uint64_t)tmp[5] << 40 | (uint64_t)tmp[4] << 32 | (uint64_t)tmp[3] << 24 | (uint64_t)tmp[2] << 16 | (uint64_t)tmp[1] <<  8 | (uint64_t)tmp[0];
}

static inline void cast_uint64_to_uint8_array_len(uint8_t *dst, uint64_t src, int len) {
    for(int i=0; i<len; i++) {
        dst[i] = (uint8_t)((src & (0x00000000000000ffULL << (8 * i)) ) >> (8 *  i));
    }
}


#endif /* __SPECK_CTR_H__ */
