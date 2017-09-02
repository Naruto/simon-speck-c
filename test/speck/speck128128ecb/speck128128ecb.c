#include <inttypes.h>
#include <speck/speck.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://eprint.iacr.org/2013/404.pdf
//
// Speck128/128
//  Key:        0f0e0d0c0b0a0908 0706050403020100
//  Plaintext:  6c61766975716520 7469206564616d20
//  Ciphertext: a65d985179783265 7860fedf5c570d18
static const uint64_t s_key[2] = {0x0706050403020100, 0x0f0e0d0c0b0a0908};
static const uint64_t s_plain_text[2] = {0x7469206564616d20, 0x6c61766975716520};
static const uint64_t s_cipher_text[2] = {0x7860fedf5c570d18, 0xa65d985179783265};
static const uint8_t s_key_stream[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};
static const uint8_t s_plain_text_stream[16] = {
    0x20, 0x6d, 0x61, 0x64, 0x65, 0x20, 0x69, 0x74, 0x20, 0x65, 0x71, 0x75, 0x69, 0x76, 0x61, 0x6c,
};
static const uint8_t s_cipher_text_stream[16] = {
    0x18, 0x0d, 0x57, 0x5c, 0xdf, 0xfe, 0x60, 0x78, 0x65, 0x32, 0x78, 0x79, 0x51, 0x98, 0x5d, 0xa6,
};

#define BLOCK_SIZE 16

int encrypt_stream_test(int block_num, const char *test_explain) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *plain_text_stream = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *expect_crypted_stream = NULL;

    plain_text_stream = malloc(BLOCK_SIZE * block_num);
    if (!plain_text_stream) {
        r = 1;
        goto finish;
    }
    crypted_text_stream = malloc(BLOCK_SIZE * block_num);
    ;
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    expect_crypted_stream = malloc(BLOCK_SIZE * block_num);
    ;
    if (!expect_crypted_stream) {
        r = 1;
        goto finish;
    }

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, SPECK_BLOCK_CIPHER_MODE_ECB, s_key_stream, sizeof(s_key_stream));
    if (!ctx) {
        r = 1;
        goto finish;
    }

    for (int i = 0; i < block_num; i++) {
        memcpy(plain_text_stream + (i * BLOCK_SIZE), s_plain_text_stream, sizeof(s_plain_text_stream));
        memcpy(expect_crypted_stream + (i * BLOCK_SIZE), s_cipher_text_stream, sizeof(s_cipher_text_stream));
    }

    r = speck_encrypt_ex(ctx, plain_text_stream, crypted_text_stream, BLOCK_SIZE * block_num);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (expect_crypted_stream[i] != crypted_text_stream[i]) {
            printf("%s idx:%d  0x%02x != 0x%02x\n", test_explain, i, expect_crypted_stream[i], crypted_text_stream[i]);
            r = 1;
            goto finish;
        }
    }

finish:
    free(plain_text_stream);
    free(crypted_text_stream);
    free(expect_crypted_stream);

    speck_finish(&ctx);
    return r;
}

int decrypt_stream_test(int block_num, const char *test_explain) {
    int r = 0;
    speck_ctx_t *ctx = NULL;
    uint8_t *crypted_text_stream = NULL;
    uint8_t *decrypted_text_stream = NULL;
    uint8_t *expect_decrypted_stream = NULL;

    crypted_text_stream = malloc(BLOCK_SIZE * block_num);
    ;
    if (!crypted_text_stream) {
        r = 1;
        goto finish;
    }
    decrypted_text_stream = malloc(BLOCK_SIZE * block_num);
    if (!decrypted_text_stream) {
        r = 1;
        goto finish;
    }
    expect_decrypted_stream = malloc(BLOCK_SIZE * block_num);
    ;
    if (!expect_decrypted_stream) {
        r = 1;
        goto finish;
    }

    ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, SPECK_BLOCK_CIPHER_MODE_ECB, s_key_stream, sizeof(s_key_stream));
    if (!ctx) {
        r = 1;
        goto finish;
    }

    for (int i = 0; i < block_num; i++) {
        memcpy(crypted_text_stream + (i * BLOCK_SIZE), s_cipher_text_stream, sizeof(s_cipher_text_stream));
        memcpy(expect_decrypted_stream + (i * BLOCK_SIZE), s_plain_text_stream, sizeof(s_plain_text_stream));
    }

    r = speck_decrypt_ex(ctx, crypted_text_stream, decrypted_text_stream, BLOCK_SIZE * block_num);
    if (r < 0) {
        r = 1;
        goto finish;
    }
    for (int i = 0; i < BLOCK_SIZE * block_num; i++) {
        if (expect_decrypted_stream[i] != decrypted_text_stream[i]) {
            printf("%s idx:%d  0x%02x != 0x%02x\n", test_explain, i, expect_decrypted_stream[i], decrypted_text_stream[i]);
            r = 1;
            goto finish;
        }
    }

finish:
    free(crypted_text_stream);
    free(decrypted_text_stream);
    free(expect_decrypted_stream);

    speck_finish(&ctx);
    return r;
}

int main() {
    int r = 0;

    // encrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, SPECK_BLOCK_CIPHER_MODE_ECB, s_key_stream, sizeof(s_key_stream));
        if (!ctx) return 1;

        uint64_t crypted_text[2];

        speck_encrypt(ctx, s_plain_text, crypted_text);
        if (!(crypted_text[0] == s_cipher_text[0] && crypted_text[1] == s_cipher_text[1])) {
            printf("not match encrypt expect:0x%" PRIu64 " 0x%" PRIu64 " encrypted:0x%" PRIu64 " 0x%" PRIu64 "\n", s_cipher_text[1], s_cipher_text[0], crypted_text[1], crypted_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }

    // decrypt test
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, SPECK_BLOCK_CIPHER_MODE_ECB, s_key_stream, sizeof(s_key_stream));
        if (!ctx) return 1;

        uint64_t decrypted_text[2];

        speck_decrypt(ctx, s_cipher_text, decrypted_text);
        if (!(decrypted_text[0] == s_plain_text[0] && decrypted_text[1] == s_plain_text[1])) {
            printf("not match decrypt expect:0x%" PRIu64 " 0x%" PRIu64 " decrypted:0x%" PRIu64 " 0x%" PRIu64 "\n", s_plain_text[1], s_plain_text[0], decrypted_text[1], decrypted_text[0]);
            return 1;
        }

        speck_finish(&ctx);
    }

    // stream a block encrypt
    r = encrypt_stream_test(1, "not match stream a block stream encrypted");
    if (r != 0) return 1;

    // stream a block decrypt
    r = decrypt_stream_test(1, "not match stream a block decrypted");
    if (r != 0) return 1;

    // stream 2 blocks encrypt
    r = encrypt_stream_test(2, "not match stream 2 blocks encrypted");
    if (r != 0) return 1;

    // stream 2 blocks decrypt
    r = decrypt_stream_test(2, "not match stream 2 blocks decrypted");
    if (r != 0) return 1;

    // stream 3 blocks encrypt
    r = encrypt_stream_test(3, "not match stream 3 blocks encrypted");
    if (r != 0) return 1;

    // stream 3 blocks decrypt
    r = decrypt_stream_test(3, "not match stream 3 blocks decrypted");
    if (r != 0) return 1;

    // stream 4 blocks encrypt
    r = encrypt_stream_test(4, "not match stream 4 blocks encrypted");
    if (r != 0) return 1;

    // stream 4 blocks decrypt
    r = decrypt_stream_test(4, "not match stream 4 blocks decrypted");
    if (r != 0) return 1;

    // stream 5 blocks encrypt
    r = encrypt_stream_test(5, "not match stream 5 blocks encrypted");
    if (r != 0) return 1;

    // stream 5 blocks decrypt
    r = decrypt_stream_test(5, "not match stream 5 blocks decrypted");
    if (r != 0) return 1;

    // stream 6 blocks encrypt
    r = encrypt_stream_test(6, "not match stream 6 blocks encrypted");
    if (r != 0) return 1;

    // stream 6 blocks decrypt
    r = decrypt_stream_test(6, "not match stream 6 blocks decrypted");
    if (r != 0) return 1;

    // stream 7 blocks encrypt
    r = encrypt_stream_test(7, "not match stream 7 blocks encrypted");
    if (r != 0) return 1;

    // stream 7 blocks decrypt
    r = decrypt_stream_test(7, "not match stream 7 blocks decrypted");
    if (r != 0) return 1;

    // stream 8 blocks encrypt
    r = encrypt_stream_test(8, "not match stream 8 blocks encrypted");
    if (r != 0) return 1;

    // stream 8 blocks decrypt
    r = decrypt_stream_test(8, "not match stream 8 blocks decrypted");
    if (r != 0) return 1;

    return 0;
}
