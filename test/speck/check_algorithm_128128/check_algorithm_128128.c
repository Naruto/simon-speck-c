#include "speck.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f,
    };

    uint8_t expect_cipher_array[16] = {
        0x18, 0x0d, 0x57, 0x5c,
        0xdf, 0xfe, 0x60, 0x78,
        0x65, 0x32, 0x78, 0x79,
        0x51, 0x98, 0x5d, 0xa8,
    };

    uint8_t expect_decrypt_array[16];
    memcpy(expect_decrypt_array, plain_text_array, sizeof(expect_decrypt_array));

    // encrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
        if(!ctx) return 1;

        uint64_t cipher_text[2];

        speck_encrypt(ctx, plain_text, cipher_text);
        // printf("0x%llx\n", cipher_text[0]);
        // printf("0x%llx\n", cipher_text[1]);
        if(!(cipher_text[0] == expect_cipher_text[0]  &&
             cipher_text[1] == expect_cipher_text[1])) {
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
        // printf("0x%llx\n", decrypt_text[0]);
        // printf("0x%llx\n", decrypt_text[1]);
        if(!(decrypt_text[0] == expect_decrypt_text[0]  &&
             decrypt_text[1] == expect_decrypt_text[1])) {
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
        if(r <0) {return 1;)}
        for(int i=0; i<16; i++) {
            printf("%0x02x == 0x02x\n", crypted_text[i], expect_cipher_array[i]);
            if(crypted_text[i] != expect_cipher_array[i])
                return 1;
        }
        // printf("\n");

        speck_finish(&ctx);
    }


    /*
    speck_decrypt_ex(ctx, crypted_text, tmp, siz);
    for(int i=siz-1;i >=0;i--)
        printf("%02x", tmp[i]);
    printf("\n");
    */


/*
    {
        int siz = 128;
        int i;
        unsigned char *plain_text = (unsigned char*)calloc(1, siz);
        unsigned char *crypted_text = (unsigned char*)calloc(1, siz);
        unsigned char *tmp_text = (unsigned char*)calloc(1, siz);

        strcpy((char*)plain_text, "abcdefghijklmnopqrstyvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        for(i=siz-1;i>=0;i--)
            printf("%02x ", plain_text[i]);
        printf("\n");

        speck_encrypt_ex(ctx, plain_text, crypted_text, siz);
        for(i=siz-1;i>=0;i--)
            printf("%02x ", crypted_text[i]);
        printf("\n");

        speck_decrypt_ex(ctx, crypted_text, tmp_text, siz);
        for(i=siz-1;i>=0;i--)
            printf("%02x ", tmp_text[i]);
        printf("\n");

        free(tmp_text);
        free(crypted_text);
        free(plain_text);
    }
*/

    return 0;
}
