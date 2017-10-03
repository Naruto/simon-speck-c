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
// Speck128/256
//  Key:        1f1e1d1c1b1a1918 1716151413121110 0f0e0d0c0b0a0908 0706050403020100
//  Plaintext:  65736f6874206e49 202e72656e6f6f70
//  Ciphertext: 4109010405c0f53e 4eeeb48d9c188f43
static const uint64_t s_plain_text[2] = {0x202e72656e6f6f70, 0x65736f6874206e49};
static const uint64_t s_cipher_text[2] = {0x4eeeb48d9c188f43, 0x4109010405c0f53e};

static const uint8_t s_key_stream[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};
static const uint8_t s_plain_text_stream[16] = {
    0x70, 0x6f, 0x6f, 0x6e, 0x65, 0x72, 0x2e, 0x20, 0x49, 0x6e, 0x20, 0x74, 0x68, 0x6f, 0x73, 0x65,
};
static const uint8_t s_cipher_text_stream[16] = {
    0x43, 0x8f, 0x18, 0x9c, 0x8d, 0xb4, 0xee, 0x4e, 0x3e, 0xf5, 0xc0, 0x05, 0x04, 0x01, 0x09, 0x41,
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

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, s_key_stream, sizeof(s_key_stream));
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
    plain_text_stream = (uint8_t*)malloc(block_num);
    if (!plain_text_stream) {
        r = 1;
        goto finish;
    }
    crypted_text_stream = (uint8_t*)malloc(block_num);
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    decrypted_text_stream = (uint8_t*)malloc(block_num);
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
    generate_random_array(plain_text_stream, block_num);
    generate_random_array(origin_iv_text_stream, BLOCK_SIZE);

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, key_text_stream, key_text_length);
    if (!ctx) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_encrypt(ctx, plain_text_stream, crypted_text_stream, block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);
    r = speck_ctr_decrypt(ctx, crypted_text_stream, decrypted_text_stream, block_num, iv_text_stream, BLOCK_SIZE);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < block_num; i++) {
        if (plain_text_stream[i] != decrypted_text_stream[i]) {
            printf("block_num:%d idx:%d  0x%02x != 0x%02x\n", block_num, i, plain_text_stream[i], decrypted_text_stream[i]);
            show_array("iv", origin_iv_text_stream, BLOCK_SIZE);
            show_array("plain", plain_text_stream, block_num);
            show_array("decrypted", decrypted_text_stream, block_num);
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
    for (int i = 0; i <(4*1024); i++) {
        int r = encrypt_decrypt_random_stream_test(i+1);
        if(r != 0) {
            return r;
        }
    }
    printf("success encrypt_decrypt_random_stream_test\n");

    return 0;
}
