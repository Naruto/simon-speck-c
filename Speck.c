//
// Created by naruto on 16/08/11.
//
#include "Speck.h"
#include <stdio.h>
#include <stdlib.h>

#if 0
//left circular shift
#define LCS _lrotl
//right circular shift
#define RCS _lrotr
#endif

struct speck_ctx_t_ {
    int mm;
    int rounds;
    int rot_a;
    int rot_b;
};

speck_ctx_t *speck_init(enum speck_encrypt_type type) {
    speck_ctx_t *ctx = calloc(sizeof(speck_ctx_t), 1);
    return ctx;
}

void speck_finish(speck_ctx_t *ctx) {
    if(!ctx) return;
    free(ctx);
}
