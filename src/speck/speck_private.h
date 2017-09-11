#ifndef __SPECK_PRIVATE_H__
#define __SPECK_PRIVATE_H__

#include <speck/speck.h>

#define ROUNDS 32
#define WORDS 8
#define BLOCK_SIZE (WORDS * 2)

struct speck_ctx_t_ {
    uint64_t key_schedule[ROUNDS];
    enum speck_encrypt_type type;
};

static inline void cast_uint8_array_to_uint64(uint64_t *dst, const uint8_t *array) {
    // TODO: byte order
    *dst = (uint64_t)array[7] << 56 | (uint64_t)array[6] << 48 | (uint64_t)array[5] << 40 | (uint64_t)array[4] << 32 | (uint64_t)array[3] << 24 | (uint64_t)array[2] << 16 | (uint64_t)array[1] << 8 | (uint64_t)array[0];
}

static inline void cast_uint64_to_uint8_array(uint8_t *dst, uint64_t src) {
    // TODO: byte order
    dst[0] = (uint8_t)(src & 0x00000000000000ff);
    dst[1] = (uint8_t)((src & 0x000000000000ff00) >> 8);
    dst[2] = (uint8_t)((src & 0x0000000000ff0000) >> 16);
    dst[3] = (uint8_t)((src & 0x00000000ff000000) >> 24);
    dst[4] = (uint8_t)((src & 0x000000ff00000000) >> 32);
    dst[5] = (uint8_t)((src & 0x0000ff0000000000) >> 40);
    dst[6] = (uint8_t)((src & 0x00ff000000000000) >> 48);
    dst[7] = (uint8_t)((src & 0xff00000000000000) >> 56);
}

int is_validate_key_len(enum speck_encrypt_type type, int key_len);

#endif /* __SPECK_PRIVATE_H__ */