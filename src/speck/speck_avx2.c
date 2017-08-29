#include <immintrin.h>
#include <stdlib.h>
#include "speck.h"
#include "speck_private.h"


#define LANE_NUM 4


struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
};

static inline void speck_round_x1(uint64_t* x, uint64_t* y, const uint64_t* k)
{
    *x = (*x >> 8) | (*x << (8 * sizeof(*x) - 8)); // x = ROTR(x, 8)
    *x += *y;
    *x ^= *k;
    *y = (*y << 3) | (*y >> (8 * sizeof(*y) - 3)); // y = ROTL(y, 3)
    *y ^= *x;
}

static inline void speck_back_x1(uint64_t* x, uint64_t* y, const uint64_t* k)
{
    *y ^= *x;
    *y = (*y >> 3) | (*y << (8 * sizeof(*y) - 3)); // y = ROTR(y, 3)
    *x ^= *k;
    *x -= *y;
    *x = (*x << 8) | (*x >> (8 * sizeof(*x) - 8)); // x = ROTL(x, 8)
}

static inline void speck_round_x4(__m256i* x, __m256i* y, const __m256i* k)
{
    *x = _mm256_xor_si256(_mm256_srli_epi64(*x,8), _mm256_slli_epi64(*x,(64-8))); // x = ROTR(x, 8)
    *x = _mm256_add_epi64(*x, *y);
    *x = _mm256_xor_si256(*x, *k);
    *y = _mm256_xor_si256(_mm256_slli_epi64(*y,3), _mm256_srli_epi64(*y,(64-3))); // y = ROTL(y, 3)
    *y = _mm256_xor_si256(*y, *x);
}

static inline void speck_back_x4(__m256i* x, __m256i * y, const __m256i* k)
{
    *y = _mm256_xor_si256(*y, *x);
    *y = _mm256_xor_si256(_mm256_srli_epi64(*y,3), _mm256_slli_epi64(*y,(64-3))); // y = ROTR(y, 3)
    *x = _mm256_xor_si256(*x, *k);
    *x = _mm256_sub_epi64(*x, *y);
    *x = _mm256_xor_si256(_mm256_slli_epi64(*x,8), _mm256_srli_epi64(*x,(64-8))); // x = ROTL(x, 8)
}

static inline void speck_encrypt_x1_inline(speck_ctx_t *ctx, uint64_t *ciphertext)
{
    for (unsigned i = 0; i < ROUNDS; i++) {
        uint64_t key = ctx->key_schedule[i];
        speck_round_x1(&ciphertext[1], &ciphertext[0], &key);
    }
}

static inline void speck_decrypt_x1_inline(speck_ctx_t *ctx, uint64_t *decrypted)
{
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

static inline void speck_decrypt_x4_inline(speck_ctx_t *ctx, __m256i *decrypted)
{
    for (unsigned i = ROUNDS; i > 0; i--) {
        __m256i key = _mm256_set1_epi64x(ctx->key_schedule[i - 1]);
        speck_back_x4(&decrypted[1], &decrypted[0], &key);
    }
}

void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2], uint64_t ciphertext[2]) {
    ciphertext[0] = plaintext[0];
    ciphertext[1] = plaintext[1];
    speck_encrypt_x1_inline(ctx, ciphertext);
}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2]) {
    decrypted[0] = ciphertext[0];
    decrypted[1] = ciphertext[1];
    speck_decrypt_x1_inline(ctx, decrypted);
}

int speck_encrypt_ex(speck_ctx_t *ctx, const unsigned char *plain, unsigned char *crypted, int plain_len) {
    if(plain_len % BLOCK_SIZE != 0) {
        return -1;
    }

    int len = plain_len / (BLOCK_SIZE * LANE_NUM);
    int remain = (plain_len % (BLOCK_SIZE * LANE_NUM)) / BLOCK_SIZE;

    int i = 0;
    int array_idx = 0;
    unsigned char *cur_plain;
    unsigned char *cur_crypted;
    uint64_t tmp_low[LANE_NUM];
    uint64_t tmp_high[LANE_NUM];
    for(i=0; i<len; i++) {
        __m256i  crypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (unsigned char *)(plain + array_idx);

        crypted_lane[0] = _mm256_set_epi64x(*((uint64_t*)(cur_plain + (WORDS * 6))), *((uint64_t*)(cur_plain + (WORDS * 4))), *((uint64_t*)(cur_plain + (WORDS * 2))), *((uint64_t*)(cur_plain + (WORDS * 0))));
        crypted_lane[1] = _mm256_set_epi64x(*((uint64_t*)(cur_plain + (WORDS * 7))), *((uint64_t*)(cur_plain + (WORDS * 5))), *((uint64_t*)(cur_plain + (WORDS * 3))), *((uint64_t*)(cur_plain + (WORDS * 1))));

        speck_encrypt_x4_inline(ctx, crypted_lane);

        cur_crypted = (unsigned char *)(crypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, crypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, crypted_lane[1]);

        ((uint64_t*)(cur_crypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_crypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_crypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_crypted + (WORDS * 3)))[0] = tmp_high[1];

        ((uint64_t*)(cur_crypted + (WORDS * 4)))[0] = tmp_low[2];
        ((uint64_t*)(cur_crypted + (WORDS * 5)))[0] = tmp_high[2];

        ((uint64_t*)(cur_crypted + (WORDS * 6)))[0] = tmp_low[3];
        ((uint64_t*)(cur_crypted + (WORDS * 7)))[0] = tmp_high[3];
    }
    if(remain == 3) {
        __m256i  crypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (unsigned char *)(plain + array_idx);

        crypted_lane[0] = _mm256_set_epi64x(0, *((uint64_t*)(cur_plain + (WORDS * 4))), *((uint64_t*)(cur_plain + (WORDS * 2))), *((uint64_t*)(cur_plain + (WORDS * 0))));
        crypted_lane[1] = _mm256_set_epi64x(0, *((uint64_t*)(cur_plain + (WORDS * 5))), *((uint64_t*)(cur_plain + (WORDS * 3))), *((uint64_t*)(cur_plain + (WORDS * 1))));

        speck_encrypt_x4_inline(ctx, crypted_lane);

        cur_crypted = (unsigned char *)(crypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, crypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, crypted_lane[1]);

        ((uint64_t*)(cur_crypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_crypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_crypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_crypted + (WORDS * 3)))[0] = tmp_high[1];

        ((uint64_t*)(cur_crypted + (WORDS * 4)))[0] = tmp_low[2];
        ((uint64_t*)(cur_crypted + (WORDS * 5)))[0] = tmp_high[2];
    }
    if(remain == 2) {
        __m256i  crypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (unsigned char *)(plain + array_idx);

        crypted_lane[0] = _mm256_set_epi64x(0, 0, *((uint64_t*)(cur_plain + (WORDS * 2))), *((uint64_t*)(cur_plain + (WORDS * 0))));
        crypted_lane[1] = _mm256_set_epi64x(0, 0, *((uint64_t*)(cur_plain + (WORDS * 3))), *((uint64_t*)(cur_plain + (WORDS * 1))));

        speck_encrypt_x4_inline(ctx, crypted_lane);

        cur_crypted = (unsigned char *)(crypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, crypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, crypted_lane[1]);

        ((uint64_t*)(cur_crypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_crypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_crypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_crypted + (WORDS * 3)))[0] = tmp_high[1];
    }
    if(remain == 1) {
        uint64_t crypted_block[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (unsigned char *)(plain + array_idx);

        crypted_block[0] = *((uint64_t *)(cur_plain + (WORDS * 0)));
        crypted_block[1] = *((uint64_t *)(cur_plain + (WORDS * 1)));

        speck_encrypt_x1_inline(ctx, crypted_block);

        cur_crypted = (unsigned char *)(crypted + array_idx);
        ((uint64_t*)(cur_crypted + (WORDS * 0)))[0] = crypted_block[0];
        ((uint64_t*)(cur_crypted + (WORDS * 1)))[0] = crypted_block[1];
    }

    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const unsigned char *crypted, unsigned char *decrypted, int crypted_len) {
    if(crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }

    int len = crypted_len / (BLOCK_SIZE * LANE_NUM);
    int remain = (crypted_len % (BLOCK_SIZE * LANE_NUM)) / BLOCK_SIZE;

    int i = 0;
    int array_idx = 0;
    unsigned char *cur_crypted;
    unsigned char *cur_decrypted;
    uint64_t tmp_low[LANE_NUM];
    uint64_t tmp_high[LANE_NUM];
    for(i=0; i<len; i++) {
        __m256i  decrypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (unsigned char *)(crypted + array_idx);

        decrypted_lane[0] = _mm256_set_epi64x(*((uint64_t*)(cur_crypted + (WORDS * 6))), *((uint64_t*)(cur_crypted + (WORDS * 4))), *((uint64_t*)(cur_crypted + (WORDS * 2))), *((uint64_t*)(cur_crypted + (WORDS * 0))));
        decrypted_lane[1] = _mm256_set_epi64x(*((uint64_t*)(cur_crypted + (WORDS * 7))), *((uint64_t*)(cur_crypted + (WORDS * 5))), *((uint64_t*)(cur_crypted + (WORDS * 3))), *((uint64_t*)(cur_crypted + (WORDS * 1))));

        speck_decrypt_x4_inline(ctx, decrypted_lane);

        cur_decrypted = (unsigned char *)(decrypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, decrypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, decrypted_lane[1]);

        ((uint64_t*)(cur_decrypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_decrypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_decrypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_decrypted + (WORDS * 3)))[0] = tmp_high[1];

        ((uint64_t*)(cur_decrypted + (WORDS * 4)))[0] = tmp_low[2];
        ((uint64_t*)(cur_decrypted + (WORDS * 5)))[0] = tmp_high[2];

        ((uint64_t*)(cur_decrypted + (WORDS * 6)))[0] = tmp_low[3];
        ((uint64_t*)(cur_decrypted + (WORDS * 7)))[0] = tmp_high[3];
    }
    if(remain == 3) {
        __m256i  decrypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (unsigned char *)(crypted + array_idx);

        decrypted_lane[0] = _mm256_set_epi64x(0, *((uint64_t*)(cur_crypted + (WORDS * 4))), *((uint64_t*)(cur_crypted + (WORDS * 2))), *((uint64_t*)(cur_crypted + (WORDS * 0))));
        decrypted_lane[1] = _mm256_set_epi64x(0, *((uint64_t*)(cur_crypted + (WORDS * 5))), *((uint64_t*)(cur_crypted + (WORDS * 3))), *((uint64_t*)(cur_crypted + (WORDS * 1))));

        speck_decrypt_x4_inline(ctx, decrypted_lane);

        cur_decrypted = (unsigned char *)(decrypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, decrypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, decrypted_lane[1]);

        ((uint64_t*)(cur_decrypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_decrypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_decrypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_decrypted + (WORDS * 3)))[0] = tmp_high[1];

        ((uint64_t*)(cur_decrypted + (WORDS * 4)))[0] = tmp_low[2];
        ((uint64_t*)(cur_decrypted + (WORDS * 5)))[0] = tmp_high[2];
    }
    if(remain == 2) {
        __m256i  decrypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (unsigned char *)(crypted + array_idx);

        decrypted_lane[0] = _mm256_set_epi64x(0, 0, *((uint64_t*)(cur_crypted + (WORDS * 2))), *((uint64_t*)(cur_crypted + (WORDS * 0))));
        decrypted_lane[1] = _mm256_set_epi64x(0, 0, *((uint64_t*)(cur_crypted + (WORDS * 3))), *((uint64_t*)(cur_crypted + (WORDS * 1))));

        speck_decrypt_x4_inline(ctx, decrypted_lane);

        cur_decrypted = (unsigned char *)(decrypted + array_idx);

        _mm256_storeu_si256((__m256i *)tmp_low, decrypted_lane[0]);
        _mm256_storeu_si256((__m256i *)tmp_high, decrypted_lane[1]);

        ((uint64_t*)(cur_decrypted + (WORDS * 0)))[0] = tmp_low[0];
        ((uint64_t*)(cur_decrypted + (WORDS * 1)))[0] = tmp_high[0];

        ((uint64_t*)(cur_decrypted + (WORDS * 2)))[0] = tmp_low[1];
        ((uint64_t*)(cur_decrypted + (WORDS * 3)))[0] = tmp_high[1];
    }
    if(remain == 1) {
        uint64_t decrypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (unsigned char *)(crypted + array_idx);

        decrypted_lane[0] = *((uint64_t *)(cur_crypted + (WORDS * 0)));
        decrypted_lane[1] = *((uint64_t *)(cur_crypted + (WORDS * 1)));

        speck_decrypt_x1_inline(ctx, decrypted_lane);

        cur_decrypted = (unsigned char *)(decrypted + array_idx);
        ((uint64_t*)(cur_decrypted + (WORDS * 0)))[0] = decrypted_lane[0];
        ((uint64_t*)(cur_decrypted + (WORDS * 1)))[0] = decrypted_lane[1];
    }


    return 0;

}

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = (speck_ctx_t *)calloc(1, sizeof(speck_ctx_t));
    if(!ctx) return NULL;

    // calc key schedule
    uint64_t b = key[0];
    uint64_t a = key[1];
    ctx->key_schedule[0] = key[0];
    for (unsigned i = 0; i < ROUNDS - 1; i++) {
        uint64_t k = i;
        speck_round_x1(&a, &b, &k);
        ctx->key_schedule[i + 1] = b;
    }

    return ctx;
}

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
