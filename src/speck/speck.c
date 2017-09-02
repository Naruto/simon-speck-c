//
// Created by naruto on 16/08/11.
//
#include <speck/speck.h>
#include <stdlib.h>
#include "speck_private.h"

struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
};

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

int speck_encrypt_ex(speck_ctx_t *ctx, const unsigned char *plain, unsigned char *crypted, int plain_len) {
    if (plain_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int len = plain_len / BLOCK_SIZE;

    int i;
    for (i = 0; i < len; i++) {
        uint64_t plain_block[2];
        uint64_t crypted_block[2];

        int array_idx = (i * BLOCK_SIZE);

        unsigned char *cur_plain = (unsigned char *)(plain + array_idx);
        cast_uint8_array_to_uint64(&plain_block[0], cur_plain);
        cast_uint8_array_to_uint64(&plain_block[1], cur_plain + WORDS);

        speck_encrypt(ctx, plain_block, crypted_block);

        unsigned char *cur_crypted = (unsigned char *)(crypted + array_idx);
        cast_uint64_to_uint8_array(cur_crypted, crypted_block[0]);
        cast_uint64_to_uint8_array(cur_crypted + WORDS, crypted_block[1]);
    }
    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const unsigned char *crypted, unsigned char *decrypted, int crypted_len) {
    if (crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int len = crypted_len / BLOCK_SIZE;

    int i;
    for (i = 0; i < len; i++) {
        uint64_t crypted_block[2];
        uint64_t decrypted_block[2];

        int array_idx = (i * BLOCK_SIZE);

        unsigned char *cur_crypted = (unsigned char *)(crypted + array_idx);
        cast_uint8_array_to_uint64(&crypted_block[0], cur_crypted);
        cast_uint8_array_to_uint64(&crypted_block[1], cur_crypted + WORDS);

        speck_decrypt(ctx, crypted_block, decrypted_block);

        unsigned char *cur_decrypted = (unsigned char *)(decrypted + array_idx);
        cast_uint64_to_uint8_array(cur_decrypted, decrypted_block[0]);
        cast_uint64_to_uint8_array(cur_decrypted + WORDS, decrypted_block[1]);
    }
    return 0;
}

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = calloc(1, sizeof(speck_ctx_t));
    if (!ctx) return NULL;

    // calc key schedule
    uint64_t b = key[0];
    uint64_t a = key[1];
    ctx->key_schedule[0] = key[0];
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
