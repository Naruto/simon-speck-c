#include <speck/speck.h>
#include <stdlib.h>
#include "speck_private.h"
#include "speck_ctr_private.h"

#define LANE_NUM 1

int speck_ctr_encrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    if (len % BLOCK_SIZE != 0) {
        return -1;
    }
    if(iv_len != BLOCK_SIZE) {
        return -1;
    }

    int count = len / (BLOCK_SIZE * LANE_NUM);

    int i;
    for (i = 0; i < count; i++) {
        uint64_t crypted_iv_block[2];
        uint64_t plain_block[2];
        uint64_t iv_block[2];

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

    return 0;
}

int speck_ctr_decrypt(speck_ctx_t *ctx, const uint8_t *in, uint8_t *out, int len, uint8_t *iv, int iv_len) {
    return speck_ctr_encrypt(ctx, in, out, len, iv, iv_len);
}
