//
// Created by Naruto TAKAHASHI on 2017/09/02.
//

#include <speck/speck.h>
#include "speck_private.h"

int is_validate_key_len(enum speck_encrypt_type type, int key_len) {
    if (type == SPECK_ENCRYPT_TYPE_128_128) {
        return (key_len == (128 / 8));
    }

    return -1;
}