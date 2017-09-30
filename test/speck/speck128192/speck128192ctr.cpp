/*
 * Copyright (c) 2016-2017 Naruto TAKAHASHI <tnaruto@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <random>
#include <speck/speck.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://eprint.iacr.org/2013/404.pdf
//
// Speck128/192
//  Key:        1716151413121110 0f0e0d0c0b0a0908 0706050403020100
//  Plaintext:  7261482066656968 43206f7420746e65
//  Ciphertext: 1be4cf3a13135566 f9bc185de03c1886
static const uint64_t s_plain_text[2] = {0x43206f7420746e65, 0x7261482066656968};
static const uint64_t s_cipher_text[2] = {0xf9bc185de03c1886, 0x1be4cf3a13135566};

static const uint8_t s_key_stream[24] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
};
static const uint8_t s_plain_text_stream[16] = {
    0x65, 0x6e, 0x74, 0x20, 0x74, 0x6f, 0x20, 0x43, 0x68, 0x69, 0x65, 0x66, 0x20, 0x48, 0x61, 0x72,
};
static const uint8_t s_cipher_text_stream[16] = {
    0x86, 0x18, 0x3c, 0xe0, 0x5d, 0x18, 0xbc, 0xf9, 0x66, 0x55, 0x13, 0x13, 0x3a, 0xcf, 0xe4, 0x1b,
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

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_192, s_key_stream, sizeof(s_key_stream));
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
    int key_text_length = sizeof(s_key_stream);
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *iv_text_stream = NULL;
    uint8_t *origin_iv_text_stream = NULL;

    key_text_stream = (uint8_t*)malloc(key_text_length);
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

    generate_random_array(key_text_stream, key_text_length);
    generate_random_array(plain_text_stream, BLOCK_SIZE * block_num);
    generate_random_array(origin_iv_text_stream, BLOCK_SIZE);

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_192, key_text_stream, key_text_length);
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
