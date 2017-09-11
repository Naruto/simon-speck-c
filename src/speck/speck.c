//
// Created by naruto on 16/08/11.
//
#include <speck/speck.h>
#include <stdbool.h>
#include <stdlib.h>
#include "speck_private.h"

#define LANE_NUM 1

static inline void speck_round(uint64_t *x, uint64_t *y, uint64_t k) {
    *x = (*x >> 8) | (*x << (8 * sizeof(*x) - 8));  // x = ROTR(x, 8)
    *x += *y;
    *x ^= k;
    *y = (*y << 3) | (*y >> (8 * sizeof(*y) - 3));  // y = ROTL(y, 3)
    *y ^= *x;
}

static inline void speck_back(uint64_t *x, uint64_t *y, uint64_t k) {
    *y ^= *x;
    *y = (*y >> 3) | (*y << (8 * sizeof(*y) - 3));  // y = ROTR(y, 3)
    *x ^= k;
    *x -= *y;
    *x = (*x << 8) | (*x >> (8 * sizeof(*x) - 8));  // x = ROTL(x, 8)
}

void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2], uint64_t ciphertext[2]) {
    ciphertext[0] = plaintext[0];
    ciphertext[1] = plaintext[1];
    for (unsigned i = 0; i < ROUNDS; i++) {
        speck_round(&ciphertext[1], &ciphertext[0], ctx->key_schedule[i]);
    }
}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2]) {
    decrypted[0] = ciphertext[0];
    decrypted[1] = ciphertext[1];
    for (unsigned i = ROUNDS; i > 0; i--) {
        speck_back(&decrypted[1], &decrypted[0], ctx->key_schedule[i - 1]);
    }
}

int speck_encrypt_ex(speck_ctx_t *ctx, const uint8_t *plain, uint8_t *crypted, int plain_len) {
    if (plain_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int count = plain_len / (BLOCK_SIZE * LANE_NUM);

    int i;
    for (i = 0; i < count; i++) {
        uint64_t plain_block[2];
        uint64_t crypted_block[2];

        int array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        uint8_t *cur_plain = (uint8_t *)(plain + array_idx);
        cast_uint8_array_to_uint64(&plain_block[0], cur_plain + (WORDS * 0));
        cast_uint8_array_to_uint64(&plain_block[1], cur_plain + (WORDS * 1));

        speck_encrypt(ctx, plain_block, crypted_block);

        uint8_t *cur_crypted = (uint8_t *)(crypted + array_idx);
        cast_uint64_to_uint8_array(cur_crypted + (WORDS * 0), crypted_block[0]);
        cast_uint64_to_uint8_array(cur_crypted + (WORDS * 1), crypted_block[1]);
    }
    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const uint8_t *crypted, uint8_t *decrypted, int crypted_len) {
    if (crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int count = crypted_len / (BLOCK_SIZE * LANE_NUM);

    int i;
    for (i = 0; i < count; i++) {
        uint64_t crypted_block[2];
        uint64_t decrypted_block[2];

        int array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        uint8_t *cur_crypted = (uint8_t *)(crypted + array_idx);
        cast_uint8_array_to_uint64(&crypted_block[0], cur_crypted + (WORDS * 0));
        cast_uint8_array_to_uint64(&crypted_block[1], cur_crypted + (WORDS * 1));

        speck_decrypt(ctx, crypted_block, decrypted_block);

        uint8_t *cur_decrypted = (uint8_t *)(decrypted + array_idx);
        cast_uint64_to_uint8_array(cur_decrypted + (WORDS * 0), decrypted_block[0]);
        cast_uint64_to_uint8_array(cur_decrypted + (WORDS * 1), decrypted_block[1]);
    }
    return 0;
}

int speck_ecb_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len) {
    return speck_encrypt_ex(ctx, in, out, len);
}

int speck_ecb_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len) {
    return speck_decrypt_ex(ctx, in, out, len);
}

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint8_t *key, int key_len) {
    if (key == NULL) return NULL;
    if (!is_validate_key_len(type, key_len)) return NULL;

    speck_ctx_t *ctx = calloc(1, sizeof(speck_ctx_t));
    if (!ctx) return NULL;
    ctx->type = type;

    // calc key schedule
    uint64_t b;
    uint64_t a;
    cast_uint8_array_to_uint64(&b, key + 0);
    cast_uint8_array_to_uint64(&a, key + 8);
    ctx->key_schedule[0] = b;
    for (unsigned i = 0; i < ROUNDS - 1; i++) {
        speck_round(&a, &b, i);
        ctx->key_schedule[i + 1] = b;
    }

    return ctx;
}

void speck_finish(speck_ctx_t **ctx) {
    if (!ctx) return;
    free(*ctx);
}
