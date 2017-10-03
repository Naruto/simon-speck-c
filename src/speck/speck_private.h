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

#ifndef __SPECK_PRIVATE_H__
#define __SPECK_PRIVATE_H__

#include <speck/speck.h>

#define ROUNDS 32
#define WORDS 8
#define BLOCK_SIZE (WORDS * 2)
#define MAX_KEY_WORDS 4

struct speck_ctx_t_ {
    int round;
    uint64_t *key_schedule;
    enum speck_encrypt_type type;
};

static inline void cast_uint8_array_to_uint64(uint64_t *dst, const uint8_t *array) {
    // TODO: byte order
    *dst = (uint64_t)array[7] << 56 | (uint64_t)array[6] << 48 | (uint64_t)array[5] << 40 | (uint64_t)array[4] << 32 | (uint64_t)array[3] << 24 | (uint64_t)array[2] << 16 | (uint64_t)array[1] << 8 | (uint64_t)array[0];
}

static inline void cast_uint64_to_uint8_array(uint8_t *dst, uint64_t src) {
    // TODO: byte order
    dst[0] = (uint8_t)(src & 0x00000000000000ffULL);
    dst[1] = (uint8_t)((src & 0x000000000000ff00ULL) >> 8);
    dst[2] = (uint8_t)((src & 0x0000000000ff0000ULL) >> 16);
    dst[3] = (uint8_t)((src & 0x00000000ff000000ULL) >> 24);
    dst[4] = (uint8_t)((src & 0x000000ff00000000ULL) >> 32);
    dst[5] = (uint8_t)((src & 0x0000ff0000000000ULL) >> 40);
    dst[6] = (uint8_t)((src & 0x00ff000000000000ULL) >> 48);
    dst[7] = (uint8_t)((src & 0xff00000000000000ULL) >> 56);
}

int is_validate_key_len(enum speck_encrypt_type type, int key_len);
int get_round_num(enum speck_encrypt_type type);
int get_key_words_num(enum speck_encrypt_type type);

#endif /* __SPECK_PRIVATE_H__ */