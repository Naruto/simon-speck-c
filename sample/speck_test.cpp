#include <speck/speck.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <random>

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
static const char *s_test_text = "abcdefghijklmnopqrstyvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstyvwxyz";

void generate_iv(uint8_t *iv, size_t iv_len) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis;

    for(int i=0; i<iv_len; i++) {
        iv[i] = static_cast<uint8_t>(dis(gen));
    }
}

void show_array(const char *explain, const uint8_t *array, size_t len) {
    printf("%20s ", explain);
    for(int i=len-1; i >= 0; i--) {
        printf("%02x ", array[i]);
    }
    printf("\n");
}

int main() {

    // block encrypt & decrypt
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
        if(!ctx) return 1;

        uint64_t plain_text[2];
        uint64_t cipher_text[2];
        uint64_t decrypted_text[2];

        plain_text[0] = 0x7469206564616d20;
        plain_text[1] = 0x6c61766975716520;

        printf("ECB block encrypt decrypt\n");

        printf("%20s0x%016llx 0x%016llx\n", "key : ", s_key[1], s_key[0]);
        printf("%20s0x%016llx 0x%016llx\n", "plain : ", plain_text[1], plain_text[0]);

        speck_encrypt(ctx, plain_text, cipher_text);
        printf("%20s0x%016llx 0x%016llx\n", "encrypted : ", cipher_text[1], cipher_text[0]);
        if(!(cipher_text[0] == s_cipher_text[0] && cipher_text[1] == s_cipher_text[1])) {
            return 1;
        }

        speck_decrypt(ctx, cipher_text, decrypted_text);
        printf("%20s0x%016llx 0x%016llx\n", "decrypted : ", decrypted_text[1], decrypted_text[0]);
        if(!(decrypted_text[0] == s_plain_text[0] && decrypted_text[1] == s_plain_text[1])) {
            return 1;
        }

        printf("\n");

        speck_finish(&ctx);
    }

    // ECB stream encrypt & decrypt
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
        if(!ctx) return 1;

        int i;
        size_t siz = 16;
        uint8_t *plain_text = (uint8_t*) calloc(1, siz);
        uint8_t *crypted_text = (uint8_t*) calloc(1, siz);
        uint8_t *decrypted_text = (uint8_t*) calloc(1, siz);

        memcpy(plain_text, s_plain_text_stream, sizeof(s_cipher_text_stream));

        printf("ECB stream ph1\n");

        show_array("plain text :", plain_text, siz);

        speck_ecb_encrypt(ctx, plain_text, crypted_text, siz);
        show_array("encrypted text :", crypted_text, siz);
        // check
        for (int i = 0; i < siz; i++) {
            if (s_cipher_text_stream[i] != crypted_text[i]) {
                printf("encrypted error idx:%d  0x%02x != 0x%02x\n", i, s_cipher_text_stream[i], crypted_text[i]);
                return 1;
            }
        }

        speck_ecb_decrypt(ctx, crypted_text, decrypted_text, siz);
        show_array("decrypted text :", decrypted_text, siz);

        // check
        for (int i = 0; i < siz; i++) {
            if (s_plain_text_stream[i] != decrypted_text[i]) {
                printf("decrypted error idx:%d  0x%02x != 0x%02x\n", i, s_plain_text_stream[i], decrypted_text[i]);
                return 1;
            }
        }
        printf("\n");

        free(decrypted_text);
        free(crypted_text);
        free(plain_text);

        speck_finish(&ctx);
    }

    // ECB stream encrypt & decrypt ph2
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
        if(!ctx) return 1;

        size_t siz = 80;
        int i;
        uint8_t *plain_text = (uint8_t*)calloc(1, siz);
        uint8_t *crypted_text = (uint8_t*)calloc(1, siz);
        uint8_t *decrypted_text = (uint8_t*)calloc(1, siz);

        printf("ECB stream ph2\n");

        memcpy(plain_text, s_test_text, strlen(s_test_text));
        show_array("plain text :", plain_text, siz);

        speck_ecb_encrypt(ctx, plain_text, crypted_text, siz);
        show_array("encrypted text :", crypted_text, siz);

        speck_ecb_decrypt(ctx, crypted_text, decrypted_text, siz);
        show_array("decrypted text :", decrypted_text, siz);

        // check
        for (int i = 0; i < siz; i++) {
            if (plain_text[i] != decrypted_text[i]) {
                printf("decrypted error idx:%d  0x%02x != 0x%02x\n", i, plain_text[i], decrypted_text[i]);
                return 1;
            }
        }
        printf("\n");

        free(decrypted_text);
        free(crypted_text);
        free(plain_text);

        speck_finish(&ctx);
    }

    // CTR stream encrypt & decrypt
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
        if(!ctx) return 1;

        int i;
        size_t siz = 16;
        size_t iv_siz = 16;
        uint8_t *plain_text = (uint8_t*) calloc(1, siz);
        uint8_t *crypted_text = (uint8_t*) calloc(1, siz);
        uint8_t *decrypted_text = (uint8_t*) calloc(1, siz);
        uint8_t *iv_text = (uint8_t*) calloc(1, iv_siz);
        uint8_t *count_iv_text = (uint8_t*) calloc(1, iv_siz);

        memcpy(plain_text, s_plain_text_stream, sizeof(s_cipher_text_stream));
        generate_iv(iv_text, iv_siz);

        printf("CTR stream ph1\n");

        show_array("iv text :", iv_text, iv_siz);

        show_array("plain text :", plain_text, siz);

        memcpy(count_iv_text, iv_text, iv_siz);
        speck_ctr_encrypt(ctx, plain_text, crypted_text, siz, count_iv_text, iv_siz);
        show_array("encrypted text :", crypted_text, siz);

        memcpy(count_iv_text, iv_text, iv_siz);
        speck_ctr_decrypt(ctx, crypted_text, decrypted_text, siz, count_iv_text, iv_siz);
        show_array("decrypted text :", decrypted_text, siz);

        // check
        for (int i = 0; i < siz; i++) {
            if (s_plain_text_stream[i] != decrypted_text[i]) {
                printf("decrypted error idx:%d  0x%02x != 0x%02x\n", i, s_plain_text_stream[i], decrypted_text[i]);
                return 1;
            }
        }
        printf("\n");

        free(decrypted_text);
        free(crypted_text);
        free(plain_text);
        free(iv_text);
        free(count_iv_text);

        speck_finish(&ctx);
    }

    // CTR stream encrypt & decrypt ph2
    {
        speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, s_key_stream, sizeof(s_key_stream));
        if(!ctx) return 1;

        size_t siz = 80;
        size_t iv_siz = 16;
        int i;
        uint8_t *plain_text = (uint8_t*)calloc(1, siz);
        uint8_t *crypted_text = (uint8_t*)calloc(1, siz);
        uint8_t *decrypted_text = (uint8_t*)calloc(1, siz);
        uint8_t *iv_text = (uint8_t*) calloc(1, iv_siz);
        uint8_t *count_iv_text = (uint8_t*) calloc(1, iv_siz);

        printf("CTR stream ph2\n");

        memcpy(plain_text, s_test_text, strlen(s_test_text));
        generate_iv(iv_text, iv_siz);

        show_array("iv text :", iv_text, iv_siz);

        show_array("plain text :", plain_text, siz);

        memcpy(count_iv_text, iv_text, iv_siz);
        speck_ctr_encrypt(ctx, plain_text, crypted_text, siz, count_iv_text, iv_siz);
        show_array("encrypted text :", crypted_text, siz);

        memcpy(count_iv_text, iv_text, 16);
        speck_ctr_decrypt(ctx, crypted_text, decrypted_text,siz, count_iv_text, iv_siz);
        show_array("decrypted text :", decrypted_text, siz);

        show_array("count iv text :", count_iv_text, iv_siz);

        // check
        for (int i = 0; i < siz; i++) {
            if (plain_text[i] != decrypted_text[i]) {
                printf("decrypted error idx:%d  0x%02x != 0x%02x\n", i, plain_text[i], decrypted_text[i]);
                return 1;
            }
        }
        printf("\n");

        free(decrypted_text);
        free(crypted_text);
        free(plain_text);
        free(iv_text);

        speck_finish(&ctx);
    }


    return 0;
}
