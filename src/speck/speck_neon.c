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
#include "speck_neon_private.h"

void speck_encrypt(speck_ctx_t *ctx, const uint64_t plaintext[2], uint64_t ciphertext[2]) {
    uint64x1_t ciphertext_x1[2];

    ciphertext_x1[0] = vld1_u64(&plaintext[0]);
    ciphertext_x1[1] = vld1_u64(&plaintext[1]);

    speck_encrypt_x1_inline(ctx, ciphertext_x1);

    vst1_u64(&ciphertext[0], ciphertext_x1[0]);
    vst1_u64(&ciphertext[1], ciphertext_x1[1]);
}

void speck_decrypt(speck_ctx_t *ctx, const uint64_t ciphertext[2], uint64_t decrypted[2]) {
    uint64x1_t decrypted_x1[2];

    decrypted_x1[0] = vld1_u64(&ciphertext[0]);
    decrypted_x1[1] = vld1_u64(&ciphertext[1]);

    speck_decrypt_x1_inline(ctx, decrypted_x1);

    vst1_u64(&decrypted[0], decrypted_x1[0]);
    vst1_u64(&decrypted[1], decrypted_x1[1]);
}

int speck_encrypt_ex(speck_ctx_t *ctx, const uint8_t *plain, uint8_t *crypted, int plain_len) {
    if (plain_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int count = plain_len / (BLOCK_SIZE * LANE_NUM);

    int remain = (plain_len % (BLOCK_SIZE * LANE_NUM)) / BLOCK_SIZE;

    int i = 0;
    int array_idx = 0;
    uint8_t *cur_plain;
    uint8_t *cur_crypted;
    for (i = 0; i < count; i++) {
        uint64x2_t crypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(plain + array_idx);

        crypted_lane[0] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 0))), vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 2))));
        crypted_lane[1] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 1))), vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 3))));

        speck_encrypt_x2_inline(ctx, crypted_lane);

        cur_crypted = (uint8_t *)(crypted + array_idx);

        vst1_u8(cur_crypted + (WORDS * 0), vreinterpret_u8_u64(vget_low_u64(crypted_lane[0])));
        vst1_u8(cur_crypted + (WORDS * 1), vreinterpret_u8_u64(vget_low_u64(crypted_lane[1])));
        vst1_u8(cur_crypted + (WORDS * 2), vreinterpret_u8_u64(vget_high_u64(crypted_lane[0])));
        vst1_u8(cur_crypted + (WORDS * 3), vreinterpret_u8_u64(vget_high_u64(crypted_lane[1])));
    }
    if (remain == 1) {
        uint64x1_t crypted_block[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_plain = (uint8_t *)(plain + array_idx);

        crypted_block[0] = vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 0)));
        crypted_block[1] = vreinterpret_u64_u8(vld1_u8(cur_plain + (WORDS * 1)));

        speck_encrypt_x1_inline(ctx, crypted_block);

        cur_crypted = (uint8_t *)(crypted + array_idx);
        vst1_u8(cur_crypted + (WORDS * 0), vreinterpret_u8_u64(crypted_block[0]));
        vst1_u8(cur_crypted + (WORDS * 1), vreinterpret_u8_u64(crypted_block[1]));
    }

    return 0;
}

int speck_decrypt_ex(speck_ctx_t *ctx, const uint8_t *crypted, uint8_t *decrypted, int crypted_len) {
    if (crypted_len % BLOCK_SIZE != 0) {
        return -1;
    }
    int count = crypted_len / (BLOCK_SIZE * LANE_NUM);

    int remain = (crypted_len % (BLOCK_SIZE * LANE_NUM)) / BLOCK_SIZE;

    int i = 0;
    int array_idx = 0;
    uint8_t *cur_crypted = (uint8_t *)(crypted);
    uint8_t *cur_decrypted = (uint8_t *)(decrypted);
    for (i = 0; i < count; i++) {
        uint64x2_t decrypted_lane[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (uint8_t *)(crypted + array_idx);
        decrypted_lane[0] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 0))), vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 2))));
        decrypted_lane[1] = vcombine_u64(vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 1))), vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 3))));

        speck_decrypt_x2_inline(ctx, decrypted_lane);

        cur_decrypted = (uint8_t *)(decrypted + array_idx);

        vst1_u8(cur_decrypted + (WORDS * 0), vreinterpret_u8_u64(vget_low_u64(decrypted_lane[0])));
        vst1_u8(cur_decrypted + (WORDS * 1), vreinterpret_u8_u64(vget_low_u64(decrypted_lane[1])));
        vst1_u8(cur_decrypted + (WORDS * 2), vreinterpret_u8_u64(vget_high_u64(decrypted_lane[0])));
        vst1_u8(cur_decrypted + (WORDS * 3), vreinterpret_u8_u64(vget_high_u64(decrypted_lane[1])));
    }
    if (remain == 1) {
        uint64x1_t decrypted_block[2];

        array_idx = (i * (BLOCK_SIZE * LANE_NUM));

        cur_crypted = (uint8_t *)(crypted + array_idx);

        decrypted_block[0] = vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 0)));
        decrypted_block[1] = vreinterpret_u64_u8(vld1_u8(cur_crypted + (WORDS * 1)));

        speck_decrypt_x1_inline(ctx, decrypted_block);

        cur_decrypted = (uint8_t *)(decrypted + array_idx);
        vst1_u8(cur_decrypted + (WORDS * 0), vreinterpret_u8_u64(decrypted_block[0]));
        vst1_u8(cur_decrypted + (WORDS * 1), vreinterpret_u8_u64(decrypted_block[1]));
    }

    return 0;
}

int speck_ecb_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len) { return speck_encrypt_ex(ctx, in, out, len); }

int speck_ecb_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len) { return speck_decrypt_ex(ctx, in, out, len); }

speck_ctx_t *speck_init(enum speck_encrypt_type type, const uint8_t *key, int key_len) {
    if (key == NULL) return NULL;
    if (!is_validate_key_len(type, key_len)) return NULL;

    speck_ctx_t *ctx = (speck_ctx_t *)calloc(1, sizeof(speck_ctx_t));
    if (!ctx) return NULL;
    ctx->type = type;
    ctx->round = get_round_num(type);

    ctx->key_schedule = calloc(1, ctx->round * sizeof(uint64_t));
    if (!ctx->key_schedule) return NULL;

    // calc key schedule
    uint64x1_t b;
    uint64x1_t a;
    int key_words_num = get_key_words_num(ctx->type);
    uint64x1_t keys[MAX_KEY_WORDS];
    for (int i = 0; i < key_words_num; i++) {
        keys[i] = vreinterpret_u64_u8(vld1_u8(key + (WORDS * i)));
    }
    vst1_u64(&ctx->key_schedule[0], keys[0]);
    for (int i = 0; i < ctx->round - 1; i++) {
        b = keys[0];
        a = keys[1];
        uint64_t k = (uint64_t)i;
        uint64x1_t vk = vld1_u64(&k);

        speck_round_x1(&a, &b, &vk);
        keys[0] = b;

        if (key_words_num != 2) {
            for (int j = 1; j < (key_words_num - 1); j++) {
                keys[j] = keys[j + 1];
            }
        }
        keys[key_words_num - 1] = a;

        vst1_u64(&ctx->key_schedule[i + 1], keys[0]);
    }

    return ctx;
}

void speck_finish(speck_ctx_t **ctx) {
    if (!ctx) return;
    free((*ctx)->key_schedule);
    free(*ctx);
}
