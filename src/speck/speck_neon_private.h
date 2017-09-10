#ifndef __SPECK_NEON_PRIVATE_H__
#define __SPECK_NEON_PRIVATE_H__

#include <speck/speck.h>
#include <stdint.h>
#include "speck_private.h"

#define LANE_NUM 2

static inline void speck_round_x1(uint64x1_t *x, uint64x1_t *y, const uint64x1_t *k) {
    *x = vsri_n_u64(vshl_n_u64(*x, (64 - 8)), *x, 8);
    *x = vadd_u64(*x, *y);
    *x = veor_u64(*x, *k);
    *y = vsri_n_u64(vshl_n_u64(*y, (3)), *y, 64 - 3);
    *y = veor_u64(*y, *x);
}

static inline void speck_back_x1(uint64x1_t *x, uint64x1_t *y, const uint64x1_t *k) {
    *y = veor_u64(*y, *x);
    *y = vsri_n_u64(vshl_n_u64(*y, (64 - 3)), *y, 3);
    *x = veor_u64(*x, *k);
    *x = vsub_u64(*x, *y);
    *x = vsri_n_u64(vshl_n_u64(*x, (8)), *x, 64 - 8);
}

static inline void speck_round_x2(uint64x2_t *x, uint64x2_t *y, const uint64x2_t *k) {
    *x = vsriq_n_u64(vshlq_n_u64(*x, (64 - 8)), *x, 8);
    *x = vaddq_u64(*x, *y);
    *x = veorq_u64(*x, *k);
    *y = vsriq_n_u64(vshlq_n_u64(*y, (3)), *y, 64 - 3);
    *y = veorq_u64(*y, *x);
}

static inline void speck_back_x2(uint64x2_t *x, uint64x2_t *y, const uint64x2_t *k) {
    *y = veorq_u64(*y, *x);
    *y = vsriq_n_u64(vshlq_n_u64(*y, (64 - 3)), *y, 3);
    *x = veorq_u64(*x, *k);
    *x = vsubq_u64(*x, *y);
    *x = vsriq_n_u64(vshlq_n_u64(*x, (8)), *x, 64 - 8);
}

static inline void speck_encrypt_x1_inline(speck_ctx_t *ctx, uint64x1_t *ciphertext) {
    for (unsigned i = 0; i < ROUNDS; i++) {
        uint64x1_t key = vld1_u64(&ctx->key_schedule[i]);
        speck_round_x1(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x1_inline(speck_ctx_t *ctx, uint64x1_t *decrypted) {
    for (unsigned i = ROUNDS; i > 0; i--) {
        uint64x1_t key = vld1_u64(&ctx->key_schedule[i - 1]);
        speck_back_x1(&decrypted[1], &decrypted[0], &key);
    }
}

static inline void speck_encrypt_x2_inline(speck_ctx_t *ctx, uint64x2_t *ciphertext) {
    for (unsigned i = 0; i < ROUNDS; i++) {
        uint64x1_t key_item = vld1_u64(&ctx->key_schedule[i]);
        uint64x2_t key = vcombine_u64(key_item, key_item);
        speck_round_x2(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x2_inline(speck_ctx_t *ctx, uint64x2_t *decrypted) {
    for (unsigned i = ROUNDS; i > 0; i--) {
        uint64x1_t key_item = vld1_u64(&ctx->key_schedule[i - 1]);
        uint64x2_t key = vcombine_u64(key_item, key_item);
        speck_back_x2(&decrypted[1], &decrypted[0], &key);
    }
}

#endif /* __SPECK_NEON_PRIVATE_H__ */