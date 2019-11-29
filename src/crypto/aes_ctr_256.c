#include <stdio.h>


#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include "aes_ctr_256.h"
#include "rijndael.h"


typedef unsigned char uc;

void aes_ctr_256(const uc* in, size_t inlen, const uc key[32], const uc iv[12], uc* out)
{
   #define AES_BLOCKSZ (16)
   #define AES_KEYBITS (256)
   
   uc aesout[AES_BLOCKSZ]; // AES output block
   unsigned long rk[RKLENGTH(AES_KEYBITS)]; // Rijndael context
   int rounds = rijndaelSetupEncrypt(rk, key, AES_KEYBITS);
   uc nonce[AES_BLOCKSZ]; // Counter is first 4 bytes, 12 bytes random initialisation vector
   for (unsigned i = 0; i < 12; ++i) {
      nonce[i + 4] = iv[i];
   }
   
   size_t offset = 0;
   uint32_t counter = 0;
   while (offset < inlen) {
      nonce[0] = (counter >> 3 * 8) & 0xFF;
      nonce[1] = (counter >> 2 * 8) & 0xFF;
      nonce[2] = (counter >> 1 * 8) & 0xFF;
      nonce[3] = (counter >> 0 * 8) & 0xFF;
      rijndaelEncrypt(rk, rounds, nonce, aesout);
      for (unsigned i = 0; (i < sizeof aesout) && (offset < inlen); ++i) {
         out[offset] = in[offset] ^ aesout[i];
         ++offset;
      }
   }
   
   memset(rk, 0, sizeof rk);
   memset(nonce, 0, sizeof nonce);
   memset(aesout, 0, sizeof aesout);
}

