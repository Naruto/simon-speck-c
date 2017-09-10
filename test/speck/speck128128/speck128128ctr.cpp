#include <random>
#include <speck/speck.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://eprint.iacr.org/2013/404.pdf
//
// Speck128/128
//  Key:        0f0e0d0c0b0a0908 0706050403020100
//  Plaintext:  6c61766975716520 7469206564616d20
//  Ciphertext: a65d985179783265 7860fedf5c570d18
static const uint64_t s_key[2] = {0x0706050403020100, 0x0f0e0d0c0b0a0908};
static const uint64_t s_plain_text[2] = {0x7469206564616d20, 0x6c61766975716520};
static const uint64_t s_cipher_text[2] = {0x7860fedf5c570d18, 0xa65d985179783265};
static const uint8_t s_key_stream[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};
static const uint8_t s_plain_text_stream[16] = {
    0x20, 0x6d, 0x61, 0x64, 0x65, 0x20, 0x69, 0x74, 0x20, 0x65, 0x71, 0x75, 0x69, 0x76, 0x61, 0x6c,
};
static const uint8_t s_cipher_text_stream[16] = {
    0x18, 0x0d, 0x57, 0x5c, 0xdf, 0xfe, 0x60, 0x78, 0x65, 0x32, 0x78, 0x79, 0x51, 0x98, 0x5d, 0xa6,
};

#define BLOCK_SIZE 16

void generate_random_array(uint8_t *iv, size_t iv_len) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis;

    for(int i=0; i<iv_len; i++) {
        iv[i] = static_cast<uint8_t>(dis(gen));
    }
}

void show_array(const char *explain, const uint8_t *array, size_t len) {
    printf("%20s ", explain);
    for(int i=len-1; i >= 0; i--) {
        printf("%02x ", array[i]);
    }
    printf("\n");
}

int encrypt_decrypt_stream_test(int block_num) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *iv_text_stream = NULL;
    uint8_t *origin_iv_text_stream = NULL;

    plain_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!plain_text_stream) {
        r = 1;
        goto finish;
    }
    crypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    decrypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!decrypted_text_stream) {
        r = 1;
        goto finish;
    }
    iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    if (!iv_text_stream) {
        r = 1;
        goto finish;
    }
    origin_iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    if (!origin_iv_text_stream) {
        r = 1;
        goto finish;
    }

    for (int i = 0; i < block_num; i++) {
        memcpy(plain_text_stream + (i * BLOCK_SIZE), s_plain_text_stream, sizeof(s_plain_text_stream));
    }
    generate_random_array(origin_iv_text_stream, BLOCK_SIZE);

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
    if (!ctx) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_encrypt(ctx, plain_text_stream, crypted_text_stream, BLOCK_SIZE * block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_decrypt(ctx, crypted_text_stream, decrypted_text_stream, BLOCK_SIZE * block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (plain_text_stream[i] != decrypted_text_stream[i]) {
            printf("block_num:%d idx:%d  0x%02x != 0x%02x\n", block_num, i, plain_text_stream[i], decrypted_text_stream[i]);
            show_array("iv", origin_iv_text_stream, BLOCK_SIZE);
            show_array("plain", plain_text_stream, block_num * BLOCK_SIZE);
            show_array("decrypted", decrypted_text_stream, block_num * BLOCK_SIZE);
            show_array("counted iv", iv_text_stream, BLOCK_SIZE);
            printf("\n");

            r = 1;
            goto finish;
        }
    }

    finish:
    free(plain_text_stream);
    free(crypted_text_stream);
    free(decrypted_text_stream);
    free(iv_text_stream);
    free(origin_iv_text_stream);

    speck_finish(&ctx);
    return r;
}

int encrypt_decrypt_random_stream_test(int block_num) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *key_text_stream = NULL;
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *iv_text_stream = NULL;
    uint8_t *origin_iv_text_stream = NULL;

    key_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    if(!key_text_stream) {
        r = 1;
        goto finish;
    }
    plain_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!plain_text_stream) {
        r = 1;
        goto finish;
    }
    crypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    decrypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!decrypted_text_stream) {
        r = 1;
        goto finish;
    }
    iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    if (!iv_text_stream) {
        r = 1;
        goto finish;
    }
    origin_iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    if (!origin_iv_text_stream) {
        r = 1;
        goto finish;
    }

    generate_random_array(key_text_stream, BLOCK_SIZE);
    generate_random_array(plain_text_stream, BLOCK_SIZE * block_num);
    generate_random_array(origin_iv_text_stream, BLOCK_SIZE);

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key_text_stream, BLOCK_SIZE);
    if (!ctx) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_encrypt(ctx, plain_text_stream, crypted_text_stream, BLOCK_SIZE * block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_decrypt(ctx, crypted_text_stream, decrypted_text_stream, BLOCK_SIZE * block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (plain_text_stream[i] != decrypted_text_stream[i]) {
            printf("block_num:%d idx:%d  0x%02x != 0x%02x\n", block_num, i, plain_text_stream[i], decrypted_text_stream[i]);
            show_array("iv", origin_iv_text_stream, BLOCK_SIZE);
            show_array("plain", plain_text_stream, block_num * BLOCK_SIZE);
            show_array("decrypted", decrypted_text_stream, block_num * BLOCK_SIZE);
            show_array("counted iv", iv_text_stream, BLOCK_SIZE);
            printf("\n");

            r = 1;
            goto finish;
        }
    }

    finish:
    free(key_text_stream);
    free(plain_text_stream);
    free(crypted_text_stream);
    free(decrypted_text_stream);
    free(iv_text_stream);
    free(origin_iv_text_stream);

    speck_finish(&ctx);
    return r;
}

int main() {
    printf("test encrypt_decrypt_stream_test\n");
    for (int i = 0; i <(4*1024)/BLOCK_SIZE; i++) {
        int r = encrypt_decrypt_stream_test(i+1);
        if(r != 0) {
            return r;
        }
    }
    printf("success encrypt_decrypt_stream_test\n");

    printf("test encrypt_decrypt_random_stream_test\n");
    for (int i = 0; i <(4*1024)/BLOCK_SIZE; i++) {
        int r = encrypt_decrypt_random_stream_test(i+1);
        if(r != 0) {
            return r;
        }
    }
    printf("success encrypt_decrypt_random_stream_test\n");

    return 0;
}
