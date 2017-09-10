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
    *x = _mm256_xor_si256(_mm256_srli_epi64(*x, 8), _mm256_slli_epi64(*x, (64 - 8)));  // x = ROTR(x, 8)
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
    *x = _mm256_xor_si256(_mm256_slli_epi64(*x, 8), _mm256_srli_epi64(*x, (64 - 8)));  // x = ROTL(x, 8)
}

static inline void speck_encrypt_x1_inline(speck_ctx_t *ctx, uint64_t *ciphertext) {
    for (unsigned i = 0; i < ROUNDS; i++) {
        uint64_t key = ctx->key_schedule[i];
        speck_round_x1(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x1_inline(speck_ctx_t *ctx, uint64_t *decrypted) {
    for (unsigned i = ROUNDS; i > 0; i--) {
        uint64_t key = ctx->key_schedule[i - 1];
        speck_back_x1(&decrypted[1], &decrypted[0], &key);
    }
}

static inline void speck_encrypt_x4_inline(speck_ctx_t *ctx, __m256i *ciphertext) {
    for (unsigned i = 0; i < ROUNDS; i++) {
        __m256i key = _mm256_set1_epi64x(ctx->key_schedule[i]);
        speck_round_x4(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x4_inline(speck_ctx_t *ctx, __m256i *decrypted) {
    for (unsigned i = ROUNDS; i > 0; i--) {
        __m256i key = _mm256_set1_epi64x(ctx->key_schedule[i - 1]);
        speck_back_x4(&decrypted[1], &decrypted[0], &key);
    }
}


#endif /* __SPECK_PRIVATE_AVX2_H__ */