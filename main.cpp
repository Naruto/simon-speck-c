#include "Speck.h"
#include <stdio.h>
#include <string.h>

int main() {
    uint64_t key[2];
    key[0] = 0x0706050403020100;
    key[1] = 0x0f0e0d0c0b0a0908;

    speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key);
    if(!ctx) return 1;

    {
      uint64_t plain_text[2];
      uint64_t cipher_text[2];
      uint64_t tmp[2];

      plain_text[0] = 0x7469206564616d20;
      plain_text[1] = 0x6c61766975716520;


      speck_encrypt(ctx, plain_text, cipher_text);
      printf("0x%llx\n", cipher_text[0]);
      printf("0x%llx\n", cipher_text[1]);

      speck_decrypt(ctx, cipher_text, tmp);
      printf("0x%llx\n", tmp[0]);
      printf("0x%llx\n", tmp[1]);
    }

    {
      unsigned char plain_text[16];
      unsigned char crypted_text[16];
      unsigned char tmp[16];

      plain_text[0] = 0x20;
      plain_text[1] = 0x6D;
      plain_text[2] = 0x61;
      plain_text[3] = 0x64;

      plain_text[4] = 0x65;
      plain_text[5] = 0x20;
      plain_text[6] = 0x69;
      plain_text[7] = 0x74;

      plain_text[8] = 0x20;
      plain_text[9] = 0x65;
      plain_text[10] = 0x71;
      plain_text[11] = 0x75;

      plain_text[12] = 0x69;
      plain_text[13] = 0x76;
      plain_text[14] = 0x61;
      plain_text[15] = 0x6C;

      speck_encrypt_ex(ctx, plain_text, sizeof(plain_text), crypted_text, sizeof(crypted_text));
      for(int i=15;i >=0;i--)
        printf("%02x", crypted_text[i]);
      printf("\n");

      speck_decrypt_ex(ctx, crypted_text, sizeof(crypted_text), tmp, sizeof(tmp));
      for(int i=15;i >=0;i--)
        printf("%02x", tmp[i]);
      printf("\n");
    }

    {
      int i;
      unsigned char plain_text[128];
      unsigned char crypted_text[128];
      unsigned char tmp_text[128];

      memset(plain_text, 0, 128);
      memset(crypted_text, 0, 128);
      memset(tmp_text, 0, 128);
      strcpy((char*)plain_text, "日本語入力してみます");
        strcpy((char*)plain_text, "abcdefghijklmnopqrstyvwxyz");
      for(i=127;i>=0;i--)
        printf("%02x ", plain_text[i]);
      printf("\n");

      speck_encrypt_ex(ctx, plain_text, 128, crypted_text, 128);
      for(i=127;i>=0;i--)
        printf("%02x ", crypted_text[i]);
      printf("\n");

      speck_decrypt_ex(ctx, crypted_text, 128, tmp_text, 128);
      for(i=127;i>=0;i--)
        printf("%02x ", tmp_text[i]);
      printf("\n");
    }

    speck_finish(&ctx);
    return 0;
}
