#include <stdint.h>

// Taken from stackoverflow (see http://stackoverflow.com/questions/3830883/cpu-cycle-count-based-profiling-in-c-c-linux-x86-64)
// Can give nonsensical results on multi-core AMD processors.
static uint64_t __attribute__((always_inline)) rdtsc() {
  uint32_t lo, hi;
  asm volatile (
                "cpuid \n" /* serializing */
                "rdtsc"
                : "=a"(lo), "=d"(hi) /* outputs */
                : "a"(0) /* inputs */
                : "%ebx", "%ecx");
  /* clobbers*/
  return ((uint64_t) lo) | (((uint64_t) hi) << 32);
}
