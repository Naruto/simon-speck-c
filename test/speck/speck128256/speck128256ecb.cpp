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

#include <inttypes.h>
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

int encrypt_stream_test(int block_num, const char *test_explain) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *expect_crypted_stream = NULL;

    plain_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!plain_text_stream) {
        r = 1;
        goto finish;
    }
    crypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    ;
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    expect_crypted_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    ;
    if (!expect_crypted_stream) {
        r = 1;
        goto finish;
    }

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, s_key_stream, sizeof(s_key_stream));
    if (!ctx) {
        r = 1;
        goto finish;
    }

    for (int i = 0; i < block_num; i++) {
        memcpy(plain_text_stream + (i * BLOCK_SIZE), s_plain_text_stream, sizeof(s_plain_text_stream));
        memcpy(expect_crypted_stream + (i * BLOCK_SIZE), s_cipher_text_stream, sizeof(s_cipher_text_stream));
    }

    r = speck_ecb_encrypt(ctx, plain_text_stream, crypted_text_stream, BLOCK_SIZE * block_num);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (expect_crypted_stream[i] != crypted_text_stream[i]) {
            printf("%s idx:%d  0x%02x != 0x%02x\n", test_explain, i, expect_crypted_stream[i], crypted_text_stream[i]);
            r = 1;
            goto finish;
        }
    }

finish:
    free(plain_text_stream);
    free(crypted_text_stream);
    free(expect_crypted_stream);

    speck_finish(&ctx);
    return r;
}

int decrypt_stream_test(int block_num, const char *test_explain) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *expect_decrypted_stream = NULL;

    crypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    ;
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    decrypted_text_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    if (!decrypted_text_stream) {
        r = 1;
        goto finish;
    }
    expect_decrypted_stream = (uint8_t*)malloc(BLOCK_SIZE * block_num);
    ;
    if (!expect_decrypted_stream) {
        r = 1;
        goto finish;
    }

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, s_key_stream, sizeof(s_key_stream));
    if (!ctx) {
        r = 1;
        goto finish;
    }

    for (int i = 0; i < block_num; i++) {
        memcpy(crypted_text_stream + (i * BLOCK_SIZE), s_cipher_text_stream, sizeof(s_cipher_text_stream));
        memcpy(expect_decrypted_stream + (i * BLOCK_SIZE), s_plain_text_stream, sizeof(s_plain_text_stream));
    }

    r = speck_ecb_decrypt(ctx, crypted_text_stream, decrypted_text_stream, BLOCK_SIZE * block_num);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (expect_decrypted_stream[i] != decrypted_text_stream[i]) {
            printf("%s idx:%d  0x%02x != 0x%02x\n", test_explain, i, expect_decrypted_stream[i], decrypted_text_stream[i]);
            r = 1;
            goto finish;
        }
    }

finish:
    free(crypted_text_stream);
    free(decrypted_text_stream);
    free(expect_decrypted_stream);

    speck_finish(&ctx);
    return r;
}

int main() {
    int r = 0;

    // encrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, s_key_stream, sizeof(s_key_stream));
        if (!ctx) return 1;

        uint64_t crypted_text[2];

        speck_encrypt(ctx, s_plain_text, crypted_text);
        if (!(crypted_text[0] == s_cipher_text[0] && crypted_text[1] == s_cipher_text[1])) {
            printf("not match encrypt expect:0x%" PRIu64 " 0x%" PRIu64 " encrypted:0x%" PRIu64 " 0x%" PRIu64 "\n", s_cipher_text[1], s_cipher_text[0], crypted_text[1], crypted_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }

    // decrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_256, s_key_stream, sizeof(s_key_stream));
        if (!ctx) return 1;

        uint64_t decrypted_text[2];

        speck_decrypt(ctx, s_cipher_text, decrypted_text);
        if (!(decrypted_text[0] == s_plain_text[0] && decrypted_text[1] == s_plain_text[1])) {
            printf("not match decrypt expect:0x%" PRIu64 " 0x%" PRIu64 " decrypted:0x%" PRIu64 " 0x%" PRIu64 "\n", s_plain_text[1], s_plain_text[0], decrypted_text[1], decrypted_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }

    for(int i = 0; i < (4*1024)/BLOCK_SIZE; i++) {
        // stream a block encrypt
        r = encrypt_stream_test(i, "not match stream a block stream encrypted");
        if (r != 0) return 1;

        // stream a block decrypt
        r = decrypt_stream_test(i, "not match stream a block decrypted");
        if (r != 0) return 1;
    }

    return 0;
}
