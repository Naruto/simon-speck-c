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
#include "rdtsc.h"

#ifndef TEST_COUNT
#define TEST_COUNT (1000000)
#endif

#ifndef TEST_BYTE_LENGTH
#define TEST_BYTE_LENGTH (1024 * 4)
#endif

#define BLOCK_SIZE (128 / 8)
#define KEY_LENGTH (128 / 8)

void generate_random_array(uint8_t *buf, size_t buf_len) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis;

    for(int i=0; i<buf_len; i++) {
        buf[i] = static_cast<uint8_t>(dis(gen));
    }
}

int compare(const void *a, const void *b) {
    return *(uint64_t*)a - *(uint64_t*)b;
}

int main() {
    uint64_t start, finish;
    uint64_t cpu_frequency;
    uint64_t result;
    uint64_t samples[TEST_COUNT];
    uint8_t *data;
    double cycles_per_byte;

    speck_ctx_t *ctx = NULL;
    uint8_t *key_text_stream = NULL;
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *iv_text_stream = NULL;
    uint8_t *origin_iv_text_stream = NULL;

    key_text_stream = (uint8_t*)malloc(KEY_LENGTH);
    plain_text_stream = (uint8_t*)malloc(TEST_BYTE_LENGTH);
    crypted_text_stream = (uint8_t*)malloc(TEST_BYTE_LENGTH);
    decrypted_text_stream = (uint8_t*)malloc(TEST_BYTE_LENGTH);
    iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);
    origin_iv_text_stream = (uint8_t*)malloc(BLOCK_SIZE);

    generate_random_array(key_text_stream, KEY_LENGTH);
    generate_random_array(plain_text_stream, TEST_BYTE_LENGTH);
    generate_random_array(origin_iv_text_stream, BLOCK_SIZE);

    if(TEST_BYTE_LENGTH % BLOCK_SIZE == 0) {
        for (int i = 0; i < TEST_COUNT; i++){

            start = rdtsc();
            ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key_text_stream, KEY_LENGTH);
            speck_ecb_encrypt(ctx, plain_text_stream, crypted_text_stream, TEST_BYTE_LENGTH);
            finish = rdtsc();
            speck_finish(&ctx);
            samples[i] = finish - start;
        }
        qsort(samples, TEST_COUNT, sizeof(uint64_t), compare);

        printf("ecb 128/128\n");
        printf("data length:%6d trycount:%8d\n", TEST_BYTE_LENGTH, TEST_COUNT);
        result = samples[(TEST_COUNT/4)*3];
        cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
        printf("quartie:%10f\n", cycles_per_byte);

        result = samples[TEST_COUNT/2];
        cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
        printf("median: %10f\n", cycles_per_byte);

        result = samples[TEST_COUNT/4];
        cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
        printf("quartie:%10f\n", cycles_per_byte);

        printf("\n");
    }

    for (int i = 0; i < TEST_COUNT; i++){

      memcpy(iv_text_stream, origin_iv_text_stream, BLOCK_SIZE);

      start = rdtsc();
      ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key_text_stream, KEY_LENGTH);
      speck_ctr_encrypt(ctx, plain_text_stream, crypted_text_stream, TEST_BYTE_LENGTH, iv_text_stream, BLOCK_SIZE);
      finish = rdtsc();
      speck_finish(&ctx);
      samples[i] = finish - start;
    }
    qsort(samples, TEST_COUNT, sizeof(uint64_t), compare);

    printf("ctr 128/128\n");
    printf("data length:%6d trycount:%8d\n", TEST_BYTE_LENGTH, TEST_COUNT);
    result = samples[(TEST_COUNT/4)*3];
    cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
    printf("quartie:%10f\n", cycles_per_byte);

    result = samples[TEST_COUNT/2];
    cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
    printf("median: %10f\n", cycles_per_byte);

    result = samples[TEST_COUNT/4];
    cycles_per_byte = 1 / ((float)(TEST_BYTE_LENGTH) / result);
    printf("quartie:%10f\n", cycles_per_byte);

    printf("\n");
    
    return 0;
}
