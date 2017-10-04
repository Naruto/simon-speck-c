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
#include "speck_neon_private.h"

#define LANE_NUM 2

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
        uint64x2_t crypted_iv_block_lane[2];
        uint64x2_t plain_block_lane[2];
        uint64x2_t out_block_lane[2];

        uint64x1_t iv_blocks[2][LANE_NUM];
        uint64_t out_blocks[2][LANE_NUM];

        for (int j = 0; j < LANE_NUM; j++) {
            iv_blocks[0][j] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 0)));
            iv_blocks[1][j] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 1)));
            ctr128_inc(iv);
        }
        crypted_iv_block_lane[0] = vcombine_u64(iv_blocks[0][0], iv_blocks[0][1]);
        crypted_iv_block_lane[1] = vcombine_u64(iv_blocks[1][0], iv_blocks[1][1]);

        speck_encrypt_x2_inline(ctx, crypted_iv_block_lane);

        int array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        uint8_t *cur_plain = (uint8_t *)(in + array_idx);
        plain_block_lane[0] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 0))), vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 2))));
        plain_block_lane[1] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 1))), vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 3))));

        out_block_lane[0] = veorq_u64(crypted_iv_block_lane[0], plain_block_lane[0]);
        out_block_lane[1] = veorq_u64(crypted_iv_block_lane[1], plain_block_lane[1]);

        uint8_t *cur_crypted = (uint8_t *)(out + array_idx);
        vst1_u8(cur_crypted + (WORDS * 0), vreinterpret_u8_u64(vget_low_u64(out_block_lane[0])));
        vst1_u8(cur_crypted + (WORDS * 1), vreinterpret_u8_u64(vget_low_u64(out_block_lane[1])));
        vst1_u8(cur_crypted + (WORDS * 2), vreinterpret_u8_u64(vget_high_u64(out_block_lane[0])));
        vst1_u8(cur_crypted + (WORDS * 3), vreinterpret_u8_u64(vget_high_u64(out_block_lane[1])));
    }
    if (remain == 1) {
        uint64x1_t in_block[2];
        uint64x1_t crypted_block[2];
        uint64x1_t out_block[2];

        crypted_block[0] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 0)));
        crypted_block[1] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 1)));
        ctr128_inc(iv);

        speck_encrypt_x1_inline(ctx, crypted_block);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(in + array_idx);
        in_block[0] = vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 0)));
        in_block[1] = vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 1)));

        out_block[0] = veor_u64(crypted_block[0], in_block[0]);
        out_block[1] = veor_u64(crypted_block[1], in_block[1]);

        cur_crypted = (uint8_t *)(out + array_idx);
        vst1_u8(cur_crypted + (WORDS * 0), vreinterpret_u8_u64(out_block[0]));
        vst1_u8(cur_crypted + (WORDS * 1), vreinterpret_u8_u64(out_block[1]));
    }
    if (remain_bytes != 0) {
        uint64x1_t in_block[2];
        uint64x1_t crypted_block[2];
        uint64x1_t out_block[2];
        uint64_t tmp_block;

        crypted_block[0] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 0)));
        crypted_block[1] = vreinterpret_u64_u8(vld1_u8(iv + (WORDS * 1)));
        ctr128_inc(iv);

        speck_encrypt_x1_inline(ctx, crypted_block);

        array_idx = (i * (BLOCK_SIZE * LANE_NUM)) + (remain * BLOCK_SIZE);

        cur_plain = (uint8_t *)(in + array_idx);
        cur_crypted = (uint8_t *)(out + array_idx);
        if (remain_bytes > WORDS) {
            in_block[0] = vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 0)));
            cast_uint8_array_to_uint64_len(&tmp_block, cur_plain + (WORDS * 1), remain_bytes - WORDS);
            in_block[1] = vld1_u64(&tmp_block);

            out_block[0] = veor_u64(crypted_block[0], in_block[0]);
            out_block[1] = veor_u64(crypted_block[1], in_block[1]);

            vst1_u8(cur_crypted + (WORDS * 0), vreinterpret_u8_u64(out_block[0]));
            vst1_u64(&tmp_block, out_block[1]);
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 1), tmp_block, remain_bytes - WORDS);
        } else {
            cast_uint8_array_to_uint64_len(&tmp_block, cur_plain + (WORDS * 0), remain_bytes);
            in_block[0] = vld1_u64(&tmp_block);

            out_block[0] = veor_u64(crypted_block[0], in_block[0]);
            vst1_u64(&tmp_block, out_block[0]);
            cast_uint64_to_uint8_array_len(cur_crypted + (WORDS * 0), tmp_block, remain_bytes);
        }
    }

    return 0;
}

int speck_ctr_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) { return speck_ctr_encrypt(ctx, in, out, len, iv, iv_len); }
