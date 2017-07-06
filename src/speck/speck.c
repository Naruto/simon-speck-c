//
// Created by naruto on 16/08/11.
//
#include "speck.h"
#include <stdlib.h>
#if defined WITH_NEON
#include "arm_neon.h"
#endif

#define ROUNDS 32
#define WORDS 8
#define BLOCK_SIZE (WORDS * 2)

struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
};

static inline void cast_uint8_array_to_uint64(uint64_t *dst, const unsigned char *array) {
    // TODO: byte order
    *dst =  (uint64_t)array[7] << 56 | (uint64_t)array[6] << 48 | (uint64_t)array[5] << 40 | (uint64_t)array[4] << 32 |
        (uint64_t)array[3] << 24 | (uint64_t)array[2] << 16 | (uint64_t)array[1] << 8  | (uint64_t)array[0];
}

static inline void cast_uint64_to_uint8_array(unsigned char *dst, uint64_t src) {
    // TODO: byte order
    dst[0] = (unsigned char)(src & 0x00000000000000ff);
    dst[1] = (unsigned char)((src & 0x000000000000ff00) >> 8);
    dst[2] = (unsigned char)((src & 0x0000000000ff0000) >> 16);
    dst[3] = (unsigned char)((src & 0x00000000ff000000) >> 24);
    dst[4] = (unsigned char)((src & 0x000000ff00000000) >> 32);
    dst[5] = (unsigned char)((src & 0x0000ff0000000000) >> 40);
    dst[6] = (unsigned char)((src & 0x00ff000000000000) >> 48);
    dst[7] = (unsigned char)((src & 0xff00000000000000) >> 56);
}

#if defined WITH_NEON

static inline void speck_round_x1(uint64x1_t* x, uint64x1_t* y, uint64x1_t k)
{
    *x = vsri_n_u64(vshl_n_u64(*x, (64-8)), *x, 8);
    *x = vadd_u64(*x, *y);
    *x = veor_u64(*x, k);
    *y = vsri_n_u64(vshl_n_u64(*y, (3)), *y, 64-3);
    *y = veor_u64(*y, *x);
}

static inline void speck_back_x1(uint64_t* x, uint64_t* y, uint64_t k)
{
    *y = veor_u64(*y, *x);
    *y = vsri_n_u64(vshl_n_u64(*y, (64-3)), *y, 3);
    *x = veor_u64(*x, k);
    *x = vsub_u64(*x, *y);
    *x = vsri_n_u64(vshl_n_u64(*x, (8)), *x, 64-8);
}


static inline void speck_encrypt_inline(speck_ctx_t *ctx, uint64x1_t ciphertext[2])
{
    for (unsigned i = 0; i < ROUNDS; i++) {
        uint64x1_t key = vld1_u64(&ctx->key_schedule[i]);
        speck_round_x1(&ciphertext[1], &ciphertext[0], key);
    }
}

static inline void speck_decrypt_inline(speck_ctx_t *ctx, uint64x1_t decrypted[2])
{
    for (unsigned i = ROUNDS; i > 0; i--) {
        uint64x1_t key = vld1_u64(&ctx->key_schedule[i - 1]);
        speck_back_x1(&decrypted[1], &decrypted[0], key);
    }
}

void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2], uint64_t ciphertext[2])
{
    uint64x1_t ciphertext_x1[2];

    ciphertext_x1[0] = vld1_u64(&plaintext[0]);
    ciphertext_x1[1] = vld1_u64(&plaintext[1]);

    speck_encrypt_inline(ctx, ciphertext_x1);

    vst1_u64(&ciphertext[0], ciphertext_x1[0]);
    vst1_u64(&ciphertext[1], ciphertext_x1[1]);
}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2])
{
    uint64x1_t decrypted_x1[2];

    decrypted_x1[0] = vld1_u64(&ciphertext[0]);
    decrypted_x1[1] = vld1_u64(&ciphertext[1]);

    speck_decrypt_inline(ctx, decrypted_x1);

    vst1_u64(&decrypted[0], decrypted_x1[0]);
    vst1_u64(&decrypted[1], decrypted_x1[1]);
}

int speck_encrypt_ex(speck_ctx_t *ctx, const unsigned char *plain, unsigned char *crypted, int plain_len) {
    if(plain_len % BLOCK_SIZE != 0) {
        return -1;
    }

    int len = plain_len / BLOCK_SIZE;

    int i;
    for(i=0; i<len; i++) {
        uint64x1_t crypted_block[2];

        int array_idx = (i * BLOCK_SIZE);

        unsigned char *cur_plain = (unsigned char *)(plain + array_idx);
        crypted_block[0] = vreinterpret_u64_u8(vld1_u8(cur_plain));
        crypted_block[1] = vreinterpret_u64_u8(vld1_u8(cur_plain + WORDS));

        speck_encrypt_inline(ctx, crypted_block);

        unsigned char *cur_crypted = (unsigned char *)(crypted + array_idx);
        vst1_u8(cur_crypted, vreinterpret_u8_u64(crypted_block[0]));
        vst1_u8(cur_crypted + WORDS, vreinterpret_u8_u64(crypted_block[1]));
    }

    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const unsigned char *crypted, unsigned char *decrypted, int crypted_len) {
    if(crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }

    int len = crypted_len / BLOCK_SIZE;

    int i;
    for(i=0; i<len; i++) {
        uint64_t decrypted_block[2];

        int array_idx = (i * BLOCK_SIZE);

        unsigned char *cur_crypted = (unsigned char *)(crypted + array_idx);
        decrypted_block[0] = vreinterpret_u64_u8(vld1_u8(cur_crypted));
        decrypted_block[1] = vreinterpret_u64_u8(vld1_u8(cur_crypted + WORDS));

        speck_decrypt_inline(ctx, decrypted_block);

        unsigned char *cur_decrypted = (unsigned char *)(decrypted + array_idx);
        vst1_u8(cur_decrypted, vreinterpret_u8_u64(decrypted_block[0]));
        vst1_u8(cur_decrypted + WORDS, vreinterpret_u8_u64(decrypted_block[1]));
    }

    return 0;
}

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = (speck_ctx_t *)calloc(1, sizeof(speck_ctx_t));
    if(!ctx) return NULL;

    // calc key schedule
    uint64x1_t b = vld1_u64(&key[0]); //= key[0];
    uint64x1_t a = vld1_u64(&key[1]); //= key[1];

    ctx->key_schedule[0] = b;
    for (unsigned i = 0; i < ROUNDS - 1; i++) {
        uint64_t idx = (uint64_t)i;
        uint64x1_t vidx = vld1_u64(&idx);
        speck_round_x1(&a, &b, vidx);
        vst1_u64(&ctx->key_schedule[i + 1], b);
    }

    return ctx;
}

#else

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

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = calloc(1, sizeof(speck_ctx_t));
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

int speck_encrypt_ex(speck_ctx_t *ctx, const unsigned char *plain, unsigned char *crypted, int plain_len) {
    if(plain_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int len = plain_len / BLOCK_SIZE;

    int i;
    for(i=0; i<len; i++) {
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
    if(crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int len = crypted_len / BLOCK_SIZE;

    int i;
    for(i=0; i<len; i++) {
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

#endif

speck_ctx_t *speck_init2(const unsigned char *key) {
    uint64_t key_tmp[2];
    cast_uint8_array_to_uint64(&key_tmp[0], key);
    cast_uint8_array_to_uint64(&key_tmp[1], key + 8);
    return speck_init(SPECK_ENCRYPT_TYPE_128_128, key_tmp);
}

void speck_finish(speck_ctx_t **ctx) {
    if(!ctx) return;
    free(*ctx);
}
