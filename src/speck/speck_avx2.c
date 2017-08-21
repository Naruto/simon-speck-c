#include <immintrin.h>
#include <stdlib.h>
#include "speck.h"
#include "speck_private.h"



struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
};


void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2], uint64_t ciphertext[2])
{
}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2])
{
}

#define LANE_NUM 4

int speck_encrypt_ex(speck_ctx_t *ctx, const unsigned char *plain, unsigned char *crypted, int plain_len) {
    if(plain_len % BLOCK_SIZE != 0) {
        return -1;
    }

    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const unsigned char *crypted, unsigned char *decrypted, int crypted_len) {
    if(crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }

    return 0;
}

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint64_t key[2]) {
    speck_ctx_t *ctx = (speck_ctx_t *)calloc(1, sizeof(speck_ctx_t));
    if(!ctx) return NULL;

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
