#include "Speck.h"

int main() {
    speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_32_64);
    if(!ctx) return 1;

    speck_finish(ctx);
    return 0;
}