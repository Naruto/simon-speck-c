#include <speck/speck.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

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
        unsigned char *plain_text = (unsigned char*) calloc(1, siz);
        unsigned char *crypted_text = (unsigned char*) calloc(1, siz);
        unsigned char *decrypted_text = (unsigned char*) calloc(1, siz);

        memcpy(plain_text, s_plain_text_stream, sizeof(s_cipher_text_stream));

        printf("ECB stream ph1\n");

        printf("%20s", "plain text : ");
        for(i=siz-1;i >=0;i--)
            printf("%02x ", plain_text[i]);
        printf("\n");

        speck_encrypt_ex(ctx, plain_text, crypted_text, siz);
        printf("%20s", "encrypted text : ");
        for(i=siz-1;i >=0;i--)
            printf("%02x ", crypted_text[i]);
        printf("\n");
        // check
        for (int i = 0; i < siz; i++) {
            if (s_cipher_text_stream[i] != crypted_text[i]) {
                printf("encrypted error idx:%d  0x%02x != 0x%02x\n", i, s_cipher_text_stream[i], crypted_text[i]);
                return 1;
            }
        }

        speck_decrypt_ex(ctx, crypted_text, decrypted_text, siz);
        printf("%20s", "decrypted text : ");
        for(int i=siz-1;i >=0;i--)
            printf("%02x ", decrypted_text[i]);
        printf("\n");

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
        unsigned char *plain_text = (unsigned char*)calloc(1, siz);
        unsigned char *crypted_text = (unsigned char*)calloc(1, siz);
        unsigned char *decrypted_text = (unsigned char*)calloc(1, siz);

        printf("ECB stream ph2\n");

        char *test = "abcdefghijklmnopqrstyvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstyvwxyz";
        memcpy(plain_text, test, strlen(test));
        printf("%20s", "plain text : ");
        for(i=siz-1;i>=0;i--)
            printf("%02x ", plain_text[i]);
        printf("\n");

        speck_encrypt_ex(ctx, plain_text, crypted_text, siz);
        printf("%20s", "encrypted text : ");
        for(i=siz-1;i>=0;i--)
            printf("%02x ", crypted_text[i]);
        printf("\n");

        speck_decrypt_ex(ctx, crypted_text, decrypted_text, siz);
        printf("%20s", "decrypted text : ");
        for(i=siz-1;i>=0;i--)
            printf("%02x ", decrypted_text[i]);
        printf("\n");

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

    return 0;
}
