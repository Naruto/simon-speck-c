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

#include <speck/speck.h>
#include <stdlib.h>
#include "speck_ctr_private.h"
#include "speck_private.h"

#define LANE_NUM 1

int speck_ctr_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    if (len < 0) {
        return -1;
    }
    if (iv_len != BLOCK_SIZE) {
        return -1;
    }

    int count = len / (BLOCK_SIZE * LANE_NUM);
    int remain_bytes = len % BLOCK_SIZE;

    int i;
    uint64_t crypted_iv_block[2];
    uint64_t plain_block[2];
    uint64_t iv_block[2];
    for (i = 0; i < count; i++) {
        cast_uint8_array_to_uint64(&iv_block[0], iv + (WORDS * 0));
        cast_uint8_array_to_uint64(&iv_block[1], iv + (WORDS * 1));
        ctr128_inc(iv);

        speck_encrypt(ctx, iv_block, crypted_iv_block);

        int array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        uint8_t *cur_plain = (uint8_t *)(in + array_idx);
        cast_uint8_array_to_uint64(&plain_block[0], cur_plain + (WORDS * 0));
        cast_uint8_array_to_uint64(&plain_block[1], cur_plain + (WORDS * 1));

        uint8_t *cur_crypted = (uint8_t *)(out + array_idx);
        cast_uint64_to_uint8_array(cur_crypted + (WORDS * 0), crypted_iv_block[0] ^ plain_block[0]);
        cast_uint64_to_uint8_array(cur_crypted + (WORDS * 1), crypted_iv_block[1] ^ plain_block[1]);
    }
    if (remain_bytes != 0) {
        cast_uint8_array_to_uint64(&iv_block[0], iv + (WORDS * 0));
        cast_uint8_array_to_uint64(&iv_block[1], iv + (WORDS * 1));
        ctr128_inc(iv);

        speck_encrypt(ctx, iv_block, crypted_iv_block);

        int array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        plain_block[0] = 0;
        plain_block[1] = 0;
        uint8_t *cur_plain = (uint8_t *)(in + array_idx);
        uint8_t *cur_crypted = (uint8_t *)(out + array_idx);
        if (remain_bytes > WORDS) {
            cast_uint8_array_to_uint64_len(&plain_block[0], cur_plain + (WORDS * 0), WORDS);
            cast_uint8_array_to_uint64_len(&plain_block[1], cur_plain + (WORDS * 1), remain_bytes - WORDS);

            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 0), crypted_iv_block[0] ^ plain_block[0], WORDS);
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 1), crypted_iv_block[1] ^ plain_block[1], remain_bytes - WORDS);
        } else {
            cast_uint8_array_to_uint64_len(&plain_block[0], cur_plain + (WORDS * 0), remain_bytes);
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 0), crypted_iv_block[0] ^ plain_block[0], remain_bytes);
        }
    }

    return 0;
}

int speck_ctr_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) { return speck_ctr_encrypt(ctx, in, out, len, iv, iv_len); }
