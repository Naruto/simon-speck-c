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
#include "speck_private.h"

int is_validate_key_len(enum speck_encrypt_type type, int key_len) {
    int ret;

    switch (type) {
        case SPECK_ENCRYPT_TYPE_128_128:
            ret = (key_len == (128 / 8));
            break;
        case SPECK_ENCRYPT_TYPE_128_192:
            ret = (key_len == (192 / 8));
            break;
        case SPECK_ENCRYPT_TYPE_128_256:
            ret = (key_len == (256 / 8));
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

int get_round_num(enum speck_encrypt_type type) {
    int ret;

    switch (type) {
        case SPECK_ENCRYPT_TYPE_128_128:
            ret = 32;
            break;
        case SPECK_ENCRYPT_TYPE_128_192:
            ret = 33;
            break;
        case SPECK_ENCRYPT_TYPE_128_256:
            ret = 34;
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

int get_key_words_num(enum speck_encrypt_type type) {
    int ret;

    switch (type) {
        case SPECK_ENCRYPT_TYPE_128_128:
            ret = 2;
            break;
        case SPECK_ENCRYPT_TYPE_128_192:
            ret = 3;
            break;
        case SPECK_ENCRYPT_TYPE_128_256:
            ret = 4;
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}