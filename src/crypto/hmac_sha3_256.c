#include <stdlib.h>
#include <string.h>
#include "hmac_sha3_256.h"
#include "Keccak-readable-and-compact.c"

void hmac_sha3_256(const unsigned char* data, size_t datalen, const unsigned char* key, size_t keylen, unsigned char* out)
{
   #define BLOCKSZ (32) // SHA3-256
   unsigned char localkey[BLOCKSZ];
   
   if (keylen > BLOCKSZ) {
      FIPS202_SHA3_256(key, keylen, localkey);
   } else {
      for (unsigned i = 0; i < keylen; ++i) {
         localkey[i] = key[i];
      }
      if (keylen < BLOCKSZ) {
         memset(localkey + keylen, 0, BLOCKSZ - keylen);
      }
   }
   
   size_t buflen = BLOCKSZ + datalen;
   unsigned char* buf = (unsigned char*)malloc(buflen);
   memcpy(buf + BLOCKSZ, data, datalen);
   
   unsigned char o_key_pad[2 * BLOCKSZ];
   unsigned char* i_key_pad = buf;
   
   for (unsigned i = 0; i < BLOCKSZ; ++i) {
      o_key_pad[i] = localkey[i] ^ 0x5C;
      i_key_pad[i] = localkey[i] ^ 0x36;
   }
   
   FIPS202_SHA3_256(buf, buflen, o_key_pad + BLOCKSZ);
   FIPS202_SHA3_256(o_key_pad, 2 * BLOCKSZ, out);
   memset(localkey, 0, BLOCKSZ);
   memset(o_key_pad, 0, sizeof o_key_pad);
   memset(buf, 0, buflen);
   free(buf);
}

