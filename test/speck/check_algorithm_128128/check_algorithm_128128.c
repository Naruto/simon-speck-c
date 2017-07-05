#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "speck.h"

// https://eprint.iacr.org/2013/404.pdf
//
// Speck128/128
//  Key:        0f0e0d0c0b0a0908 0706050403020100
//  Plaintext:  6c61766975716520 7469206564616d20
//  Ciphertext: a65d985179783265 7860fedf5c570d18
int main() {
    uint64_t key[2];
    key[0] = 0x0706050403020100;
    key[1] = 0x0f0e0d0c0b0a0908;

    uint64_t plain_text[2];
    plain_text[0] = 0x7469206564616d20;
    plain_text[1] = 0x6c61766975716520;

    uint64_t expect_cipher_text[2];
    expect_cipher_text[0] = 0x7860fedf5c570d18;
    expect_cipher_text[1] = 0xa65d985179783265;

    uint64_t expect_decrypt_text[2];
    expect_decrypt_text[0] = plain_text[0];
    expect_decrypt_text[1] = plain_text[1];

    uint8_t plain_text_array[16] = {
        0x20, 0x6d, 0x61, 0x64,
        0x65, 0x20, 0x69, 0x74,
        0x20, 0x65, 0x71, 0x75,
        0x69, 0x76, 0x61, 0x6c,
    };

    uint8_t expect_cipher_array[16] = {
        0x18, 0x0d, 0x57, 0x5c,
        0xdf, 0xfe, 0x60, 0x78,
        0x65, 0x32, 0x78, 0x79,
        0x51, 0x98, 0x5d, 0xa6,
    };

    uint8_t expect_decrypt_array[16];
    memcpy(expect_decrypt_array, plain_text_array, sizeof(expect_decrypt_array));

    // encrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
        if(!ctx) return 1;

        uint64_t cipher_text[2];

        speck_encrypt(ctx, plain_text, cipher_text);
        if(!(cipher_text[0] == expect_cipher_text[0]  &&
             cipher_text[1] == expect_cipher_text[1])) {
            printf("not match encrypt expect:0x%llx 0x%llx encrypted:0x%llx 0x%llx\n", expect_cipher_text[1], expect_cipher_text[0], cipher_text[1], cipher_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }

    // decrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
        if(!ctx) return 1;

        uint64_t decrypt_text[2];

        speck_decrypt(ctx, expect_cipher_text, decrypt_text);
        if(!(decrypt_text[0] == expect_decrypt_text[0]  &&
             decrypt_text[1] == expect_decrypt_text[1])) {
            printf("not match decrypt expect:0x%llx 0x%llx decrypted:0x%llx 0x%llx\n", expect_decrypt_text[1], expect_decrypt_text[0], decrypt_text[1], decrypt_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }


    // stream encrypt
    {
        int r;
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
        if(!ctx) return 1;
        uint8_t crypted_text[16];

        r = speck_encrypt_ex(ctx, plain_text_array, crypted_text, 16);
        if(r <0) {return 1; }
        for(int i=0; i<16; i++) {
            if(crypted_text[i] != expect_cipher_array[i]) {
                printf("not match encrypted idx:%d  0x%02x != 0x%02x\n", i, crypted_text[i], expect_cipher_array[i]);
                return 1;
            }
        }

        speck_finish(&ctx);
    }

    // stream decrypt
    {
        int r;
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
        if(!ctx) return 1;
        uint8_t decrypted_array[16];

        r = speck_decrypt_ex(ctx, expect_cipher_array, decrypted_array, 16);
        if(r <0) {return 1; }
        for(int i=0; i<16; i++) {
            if(decrypted_array[i] != expect_decrypt_array[i]) {
                printf("not match decrypted idx:%d  0x%02x != 0x%02x\n", i, decrypted_array[i], expect_cipher_array[i]);
                return 1;
            }
        }

        speck_finish(&ctx);
    }

    return 0;
}
