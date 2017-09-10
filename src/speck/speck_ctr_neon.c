#include <speck/speck.h>
#include <stdlib.h>
#include "speck_neon_private.h"
#include "speck_ctr_private.h"


#define LANE_NUM 2

int speck_ctr_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    if (len % BLOCK_SIZE != 0) {
        return -1;
    }
    if(iv_len != BLOCK_SIZE) {
        return -1;
    }

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

        for(int j=0; j<LANE_NUM; j++) {
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
    if(remain == 1) {
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

    return 0;
}

int speck_ctr_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    return speck_ctr_encrypt(ctx, in, out, len, iv, iv_len);
}
