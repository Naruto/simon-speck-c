#ifndef __SPECK_PRIVATE_H__
#define __SPECK_PRIVATE_H__

#define ROUNDS 32
#define WORDS 8
#define BLOCK_SIZE (WORDS * 2)

static inline void cast_uint8_array_to_uint64(uint64_t *dst, const unsigned char *array) {
    // TODO: byte order
    *dst = (uint64_t)array[7] << 56 | (uint64_t)array[6] << 48 | (uint64_t)array[5] << 40 | (uint64_t)array[4] << 32 | (uint64_t)array[3] << 24 | (uint64_t)array[2] << 16 | (uint64_t)array[1] << 8 | (uint64_t)array[0];
}

static inline void cast_uint64_to_uint8_array(unsigned char *dst, uint64_t src) {
    // TODO: byte order
    dst[0] = (unsigned char)(src & 0x00000000000000ff);
    dst[1] = (unsigned char)((src & 0x000000000000ff00) >> 8);
    dst[2] = (unsigned char)((src & 0x0000000000ff0000) >> 16);
    dst[3] = (unsigned char)((src & 0x00000000ff000000) >> 24);
    dst[4] = (unsigned char)((src & 0x000000ff00000000) >> 32);
    dst[5] = (unsigned char)((src & 0x0000ff0000000000) >> 40);
    dst[6] = (unsigned char)((src & 0x00ff000000000000) >> 48);
    dst[7] = (unsigned char)((src & 0xff00000000000000) >> 56);
}

#endif /* __SPECK_PRIVATE_H__ */