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

#ifndef __SPECK_PRIVATE_AVX2_H__
#define __SPECK_PRIVATE_AVX2_H__

#include <immintrin.h>
#include <speck/speck.h>
#include <stdint.h>
#include "speck_private.h"

#define LANE_NUM 4

static inline void speck_round_x1(uint64_t *x, uint64_t *y, const uint64_t *k) {
    *x = (*x >> 8) | (*x << (8 * sizeof(*x) - 8));  // x = ROTR(x, 8)
    *x += *y;
    *x ^= *k;
    *y = (*y << 3) | (*y >> (8 * sizeof(*y) - 3));  // y = ROTL(y, 3)
    *y ^= *x;
}

static inline void speck_back_x1(uint64_t *x, uint64_t *y, const uint64_t *k) {
    *y ^= *x;
    *y = (*y >> 3) | (*y << (8 * sizeof(*y) - 3));  // y = ROTR(y, 3)
    *x ^= *k;
    *x -= *y;
    *x = (*x << 8) | (*x >> (8 * sizeof(*x) - 8));  // x = ROTL(x, 8)
}

static inline void speck_round_x4(__m256i *x, __m256i *y, const __m256i *k) {
    *x = _mm256_shuffle_epi8(*x, _mm256_set_epi64x(0x080f0e0d0c0b0a09LL,0x0007060504030201LL,0x080f0e0d0c0b0a09LL,0x0007060504030201LL)); // x = ROTR(x, 8)
    *x = _mm256_add_epi64(*x, *y);
    *x = _mm256_xor_si256(*x, *k);
    *y = _mm256_xor_si256(_mm256_slli_epi64(*y, 3), _mm256_srli_epi64(*y, (64 - 3)));  // y = ROTL(y, 3)
    *y = _mm256_xor_si256(*y, *x);
}

static inline void speck_back_x4(__m256i *x, __m256i *y, const __m256i *k) {
    *y = _mm256_xor_si256(*y, *x);
    *y = _mm256_xor_si256(_mm256_srli_epi64(*y, 3), _mm256_slli_epi64(*y, (64 - 3)));  // y = ROTR(y, 3)
    *x = _mm256_xor_si256(*x, *k);
    *x = _mm256_sub_epi64(*x, *y);
    *x =  _mm256_shuffle_epi8(*x, _mm256_set_epi64x(0x0e0d0c0b0a09080fLL,0x0605040302010007LL,0x0e0d0c0b0a09080fLL,0x0605040302010007LL)); // x = ROTL(x, 8)
}

static inline void speck_encrypt_x1_inline(speck_ctx_t *ctx, uint64_t *ciphertext) {
    for (int i = 0; i < ctx->round; i++) {
        uint64_t key = ctx->key_schedule[i];
        speck_round_x1(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x1_inline(speck_ctx_t *ctx, uint64_t *decrypted) {
    for (int i = ctx->round; i > 0; i--) {
        uint64_t key = ctx->key_schedule[i - 1];
        speck_back_x1(&decrypted[1], &decrypted[0], &key);
    }
}

static inline void speck_encrypt_x4_inline(speck_ctx_t *ctx, __m256i *ciphertext) {
    for (int i = 0; i < ctx->round; i++) {
        __m256i key = _mm256_set1_epi64x(ctx->key_schedule[i]);
        speck_round_x4(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x4_inline(speck_ctx_t *ctx, __m256i *decrypted) {
    for (int i = ctx->round; i > 0; i--) {
        __m256i key = _mm256_set1_epi64x(ctx->key_schedule[i - 1]);
        speck_back_x4(&decrypted[1], &decrypted[0], &key);
    }
}

#endif /* __SPECK_PRIVATE_AVX2_H__ */