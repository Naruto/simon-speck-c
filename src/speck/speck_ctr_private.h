#ifndef __SPECK_CTR_H__
#define __SPECK_CTR_H__

#include <stdint.h>

/* increment counter (64-bit int) by 1 */
static inline uint8_t *ctr128_inc(uint8_t counter[16]) {
  uint32_t n=16;
  uint8_t  c;
  do {
    --n;
    c = counter[n];
    ++c;
    counter[n] = c;
    if (c) return counter;
  } while (n);

  return counter;
}

#endif /* __SPECK_CTR_H__ */
