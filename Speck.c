//
// Created by naruto on 16/08/11.
//
#include "Speck.h"
#include <stdio.h>
#include <stdlib.h>

#define ROUNDS 32

static inline void speck_round(uint64_t* x, uint64_t* y, uint64_t k)
{
  *x = (*x >> 8) | (*x << (8 * sizeof(*x) - 8)); // x = ROTR(x, 8)
  *x += *y;
  *x ^= k;
  *y = (*y << 3) | (*y >> (8 * sizeof(*y) - 3)); // y = ROTL(y, 3)
  *y ^= *x;

}

static inline void speck_back(uint64_t* x, uint64_t* y, uint64_t k)
{
  *y ^= *x;
  *y = (*y >> 3) | (*y << (8 * sizeof(*y) - 3)); // y = ROTR(y, 3)
  *x ^= k;
  *x -= *y;
  *x = (*x << 8) | (*x >> (8 * sizeof(*x) - 8)); // x = ROTL(x, 8)

}

struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
};

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = calloc(sizeof(speck_ctx_t), 1);
    if(!ctx) return NULL;

    // calc key schedule
    uint64_t b = key[0];
    uint64_t a = key[1];
    ctx->key_schedule[0] = b;
    for (unsigned i = 0; i < ROUNDS - 1; i++) {
      speck_round(&a, &b, i);
      ctx->key_schedule[i + 1] = b;
    }

    return ctx;
}

void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2],uint64_t ciphertext[2])
{
  ciphertext[0] = plaintext[0];
  ciphertext[1] = plaintext[1];
  for (unsigned i = 0; i < ROUNDS; i++) {
    speck_round(&ciphertext[1], &ciphertext[0], ctx->key_schedule[i]);
  }

}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2])
{
  decrypted[0] = ciphertext[0];
  decrypted[1] = ciphertext[1];
  for (unsigned i = ROUNDS; i > 0; i--) {
    speck_back(&decrypted[1], &decrypted[0], ctx->key_schedule[i - 1]);
      
  }

}

void speck_finish(speck_ctx_t **ctx) {
    if(!ctx) return;
    free(*ctx);
}
