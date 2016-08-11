//
// Created by naruto on 16/08/11.
//
#include "Speck.h"

#define NUM 10

struct Parameter {
    u32 block_size;
    u32 key_size;
    u32 word_size;
    u32 key_words;
    u32 rot_a;
    u32 rot_b;
    u32 rounds;
};

static struct Parameter SpeckParameters[NUM] = {
        {
                .block_size = 32,
                .key_size   = 64,
                .word_size  = 16,
                .key_words  =  4,
                .rot_a      =  7,
                .rot_b      =  2,
                .rounds     = 22,
        },
        {
                .block_size = 48,
                .key_size   = 72,
                .word_size  = 24,
                .key_words  =  3,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 22,
        },
        {
                .block_size = 48,
                .key_size   = 96,
                .word_size  = 24,
                .key_words  =  4,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 23,
        },
        {
                .block_size = 64,
                .key_size   = 96,
                .word_size  = 32,
                .key_words  =  3,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 26,
        },
        {
                .block_size = 64,
                .key_size   = 128,
                .word_size  = 32,
                .key_words  =  4,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 27,
        },
        {
                .block_size = 96,
                .key_size   = 96,
                .word_size  = 48,
                .key_words  =  2,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 28,
        },
        {
                .block_size = 96,
                .key_size   = 144,
                .word_size  = 48,
                .key_words  =  3,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 29,
        },
        {
                .block_size = 128,
                .key_size   = 128,
                .word_size  = 64,
                .key_words  =  2,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 32,
        },
        {
                .block_size = 128,
                .key_size   = 192,
                .word_size  = 64,
                .key_words  =  3,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     = 33,
        },
        {
                .block_size = 128,
                .key_size   = 256,
                .word_size  = 64,
                .key_words  =  4,
                .rot_a      =  8,
                .rot_b      =  3,
                .rounds     =  34,
        },
};

static int speck_idx;

bool Speck_init(enum spec idx) {
    speck_idx = (int)idx;
    
}