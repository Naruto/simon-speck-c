#include "Speck.h"
#include <stdio.h>

int main() {
    uint64_t key[2];
    uint64_t plain_text[2];
    uint64_t cipher_text[2];
    uint64_t tmp[2];

    key[0] = 0x0706050403020100;
    key[1] = 0x0f0e0d0c0b0a0908;
    plain_text[0] = 0x7469206564616d20;
    plain_text[1] = 0x6c61766975716520;

    speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
    if(!ctx) return 1;

    speck_encrypt(ctx, plain_text, cipher_text);
    printf("0x%llx\n", cipher_text[0]);
    printf("0x%llx\n", cipher_text[1]);

    speck_decrypt(ctx, cipher_text, tmp);
    printf("0x%llx\n", tmp[0]);
    printf("0x%llx\n", tmp[1]);

    speck_finish(&ctx);
    return 0;
}