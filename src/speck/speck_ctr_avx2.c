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
#include "speck_avx2_private.h"
#include "speck_ctr_private.h"

#define LANE_NUM 4

int speck_ctr_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    if (len < 0) {
        return -1;
    }
    if (iv_len != BLOCK_SIZE) {
        return -1;
    }

    int remain_bytes = len % BLOCK_SIZE;
    int count = len / (BLOCK_SIZE * LANE_NUM);
    int remain = (len % (BLOCK_SIZE * LANE_NUM)) / BLOCK_SIZE;

    int i;
    uint8_t *cur_plain;
    uint8_t *cur_crypted;
    int array_idx;
    for (i = 0; i < count; i++) {
        __m256i tmp_lane[2];
        __m256i crypted_iv_block_lane[2];
        __m256i in_block_lane[2];

        uint64_t iv_blocks[2][LANE_NUM];
        uint64_t out_blocks[2][LANE_NUM];

        for (int j = 0; j < LANE_NUM; j++) {
            iv_blocks[0][j] = *((uint64_t *)(iv + (WORDS * 0)));
            iv_blocks[1][j] = *((uint64_t *)(iv + (WORDS * 1)));
            ctr128_inc(iv);
        }
        crypted_iv_block_lane[0] = _mm256_set_epi64x(iv_blocks[0][3], iv_blocks[0][1], iv_blocks[0][2], iv_blocks[0][0]);
        crypted_iv_block_lane[1] = _mm256_set_epi64x(iv_blocks[1][3], iv_blocks[1][1], iv_blocks[1][2], iv_blocks[1][0]);

        speck_encrypt_x4_inline(ctx, crypted_iv_block_lane);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(in + array_idx);
        in_block_lane[0] = _mm256_set_epi64x(*((uint64_t *)(cur_plain + (WORDS * 6))), *((uint64_t *)(cur_plain + (WORDS * 2))), *((uint64_t *)(cur_plain + (WORDS * 4))), *((uint64_t *)(cur_plain + (WORDS * 0))));
        in_block_lane[1] = _mm256_set_epi64x(*((uint64_t *)(cur_plain + (WORDS * 7))), *((uint64_t *)(cur_plain + (WORDS * 3))), *((uint64_t *)(cur_plain + (WORDS * 5))), *((uint64_t *)(cur_plain + (WORDS * 1))));

        tmp_lane[0] = _mm256_xor_si256(crypted_iv_block_lane[0], in_block_lane[0]);
        tmp_lane[1] = _mm256_xor_si256(crypted_iv_block_lane[1], in_block_lane[1]);

        cur_crypted = (uint8_t *)(out + array_idx);
        _mm256_storeu_si256((__m256i *)(cur_crypted + WORDS * (LANE_NUM * 0)), _mm256_unpacklo_epi64(tmp_lane[0], tmp_lane[1]));
        _mm256_storeu_si256((__m256i *)(cur_crypted + WORDS * (LANE_NUM * 1)), _mm256_unpackhi_epi64(tmp_lane[0], tmp_lane[1]));
    }
    if (remain == 3) {
        __m256i crypted_iv_block_lane[2];
        __m256i in_block_lane[2];
        __m256i out_block_lane[2];

        uint64_t iv_blocks[2][LANE_NUM];
        uint64_t out_blocks[2][LANE_NUM];

        for (int j = 0; j < 3; j++) {
            iv_blocks[0][j] = *((uint64_t *)(iv + (WORDS * 0)));
            iv_blocks[1][j] = *((uint64_t *)(iv + (WORDS * 1)));
            ctr128_inc(iv);
        }
        crypted_iv_block_lane[0] = _mm256_set_epi64x(0, iv_blocks[0][2], iv_blocks[0][1], iv_blocks[0][0]);
        crypted_iv_block_lane[1] = _mm256_set_epi64x(0, iv_blocks[1][2], iv_blocks[1][1], iv_blocks[1][0]);

        speck_encrypt_x4_inline(ctx, crypted_iv_block_lane);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(in + array_idx);

        in_block_lane[0] = _mm256_set_epi64x(0, *((uint64_t *)(cur_plain + (WORDS * 4))), *((uint64_t *)(cur_plain + (WORDS * 2))), *((uint64_t *)(cur_plain + (WORDS * 0))));
        in_block_lane[1] = _mm256_set_epi64x(0, *((uint64_t *)(cur_plain + (WORDS * 5))), *((uint64_t *)(cur_plain + (WORDS * 3))), *((uint64_t *)(cur_plain + (WORDS * 1))));

        out_block_lane[0] = _mm256_xor_si256(crypted_iv_block_lane[0], in_block_lane[0]);
        out_block_lane[1] = _mm256_xor_si256(crypted_iv_block_lane[1], in_block_lane[1]);

        _mm256_storeu_si256((__m256i *)&out_blocks[0], out_block_lane[0]);
        _mm256_storeu_si256((__m256i *)&out_blocks[1], out_block_lane[1]);

        cur_crypted = (uint8_t *)(out + array_idx);
        ((uint64_t *)(cur_crypted + (WORDS * 0)))[0] = out_blocks[0][0];
        ((uint64_t *)(cur_crypted + (WORDS * 1)))[0] = out_blocks[1][0];

        ((uint64_t *)(cur_crypted + (WORDS * 2)))[0] = out_blocks[0][1];
        ((uint64_t *)(cur_crypted + (WORDS * 3)))[0] = out_blocks[1][1];

        ((uint64_t *)(cur_crypted + (WORDS * 4)))[0] = out_blocks[0][2];
        ((uint64_t *)(cur_crypted + (WORDS * 5)))[0] = out_blocks[1][2];
    }
    if (remain == 2) {
        __m256i crypted_iv_block_lane[2];
        __m256i in_block_lane[2];
        __m256i out_block_lane[2];

        uint64_t iv_blocks[2][LANE_NUM];
        uint64_t out_blocks[2][LANE_NUM];

        for (int j = 0; j < 2; j++) {
            iv_blocks[0][j] = *((uint64_t *)(iv + (WORDS * 0)));
            iv_blocks[1][j] = *((uint64_t *)(iv + (WORDS * 1)));
            ctr128_inc(iv);
        }
        crypted_iv_block_lane[0] = _mm256_set_epi64x(0, 0, iv_blocks[0][1], iv_blocks[0][0]);
        crypted_iv_block_lane[1] = _mm256_set_epi64x(0, 0, iv_blocks[1][1], iv_blocks[1][0]);

        speck_encrypt_x4_inline(ctx, crypted_iv_block_lane);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(in + array_idx);

        in_block_lane[0] = _mm256_set_epi64x(0, 0, *((uint64_t *)(cur_plain + (WORDS * 2))), *((uint64_t *)(cur_plain + (WORDS * 0))));
        in_block_lane[1] = _mm256_set_epi64x(0, 0, *((uint64_t *)(cur_plain + (WORDS * 3))), *((uint64_t *)(cur_plain + (WORDS * 1))));

        out_block_lane[0] = _mm256_xor_si256(crypted_iv_block_lane[0], in_block_lane[0]);
        out_block_lane[1] = _mm256_xor_si256(crypted_iv_block_lane[1], in_block_lane[1]);

        _mm256_storeu_si256((__m256i *)&out_blocks[0], out_block_lane[0]);
        _mm256_storeu_si256((__m256i *)&out_blocks[1], out_block_lane[1]);

        cur_crypted = (uint8_t *)(out + array_idx);
        ((uint64_t *)(cur_crypted + (WORDS * 0)))[0] = out_blocks[0][0];
        ((uint64_t *)(cur_crypted + (WORDS * 1)))[0] = out_blocks[1][0];

        ((uint64_t *)(cur_crypted + (WORDS * 2)))[0] = out_blocks[0][1];
        ((uint64_t *)(cur_crypted + (WORDS * 3)))[0] = out_blocks[1][1];
    }
    if (remain == 1) {
        uint64_t in_block[2];
        uint64_t crypted_block[2];

        crypted_block[0] = *((uint64_t *)(iv + (WORDS * 0)));
        crypted_block[1] = *((uint64_t *)(iv + (WORDS * 1)));
        ctr128_inc(iv);

        speck_encrypt_x1_inline(ctx, crypted_block);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(in + array_idx);
        in_block[0] = *((uint64_t *)(cur_plain + (WORDS * 0)));
        in_block[1] = *((uint64_t *)(cur_plain + (WORDS * 1)));

        cur_crypted = (uint8_t *)(out + array_idx);
        ((uint64_t *)(cur_crypted + (WORDS * 0)))[0] = crypted_block[0] ^ in_block[0];
        ((uint64_t *)(cur_crypted + (WORDS * 1)))[0] = crypted_block[1] ^ in_block[1];
    }

    if (remain_bytes != 0) {
        uint64_t in_block[2] = {0};
        uint64_t crypted_block[2] = {0};

        crypted_block[0] = *((uint64_t *)(iv + (WORDS * 0)));
        crypted_block[1] = *((uint64_t *)(iv + (WORDS * 1)));
        ctr128_inc(iv);

        speck_encrypt_x1_inline(ctx, crypted_block);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM)) + (remain * BLOCK_SIZE);

        cur_plain = (uint8_t *)(in + array_idx);
        cur_crypted = (uint8_t *)(out + array_idx);
        if (remain_bytes > WORDS) {
            in_block[0] = *((uint64_t *)(cur_plain + (WORDS * 0)));
            cast_uint8_array_to_uint64_len(&in_block[1], cur_plain + (WORDS * 1), remain_bytes - WORDS);

            ((uint64_t *)(cur_crypted + (WORDS * 0)))[0] = crypted_block[0] ^ in_block[0];
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 1), crypted_block[1] ^ in_block[1], remain_bytes - WORDS);
        } else {
            cast_uint8_array_to_uint64_len(&in_block[0], cur_plain + (WORDS * 0), remain_bytes);
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 0), crypted_block[0] ^ in_block[0], remain_bytes);
        }
    }

    return 0;
}

int speck_ctr_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) { return speck_ctr_encrypt(ctx, in, out, len, iv, iv_len); }
