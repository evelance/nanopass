#include <stdlib.h>
#include <string.h>
#include "pbkdf.h"


int pbkdf(
   const unsigned char* password,
   size_t pwlen,
   const unsigned char* salt,
   size_t slen,
   unsigned char* key,
   size_t keylen,
   unsigned rounds,
   void (*hmac)(const unsigned char* data, size_t datalen, const unsigned char* key, size_t keylen, unsigned char* out),
   size_t diglen
)
{
   unsigned char *buf, *d1, *d2, *d3;
   unsigned c = 1, i, j;
   size_t r = diglen;
   if (!rounds || !keylen || !slen || !diglen || (buf = (unsigned char*)malloc(slen + 4 + 3 * diglen)) == NULL)
      return -1;
   d1 = buf + slen + 4;
   d2 = d1 + diglen;
   d3 = d2 + diglen;
   memcpy(buf, salt, slen);
   while (keylen > 0) {
      buf[slen + 0] = (c >> 3 * 8) & 0xFF;
      buf[slen + 1] = (c >> 2 * 8) & 0xFF;
      buf[slen + 2] = (c >> 1 * 8) & 0xFF;
      buf[slen + 3] = (c >> 0 * 8) & 0xFF;
      hmac(buf, slen + 4, password, pwlen, d2);
      memcpy(d1, d2, diglen);
      for (i = 1; i < rounds; ++i) {
         hmac(d2, diglen, password, pwlen, d3);
         memcpy(d2, d3, diglen);
         for (j = 0; j < diglen; ++j)
            d1[j] ^= d2[j];
      }
      if (keylen < diglen)
         r = keylen;
      memcpy(key, d1, r);
      key += r;
      keylen -= r;
      ++c;
   }
   memset(buf, 0, slen + 4 + 3 * diglen);
   free(buf);
   return 0;
}

