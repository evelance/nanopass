/*
   HMAC-SHA3 (KECCAK)
   This is the Keyed-Hash Message Authentication Code
   based on SHA3 (Keccak) and is used to generate a MAC,
   i.e. sign data with a key to provide the authenticity.
   
   Parameters:
   
      data - Data to sign (length-terminated).
      key  - Key to sign the data (length-terminated).
      out  - Pointer to at least 32 bytes to output the MAC.
   
*/
#ifndef __HMAC_SHA3_256_H
#define __HMAC_SHA3_256_H

#define HMAC_SHA3_DIGLEN (32)

void hmac_sha3_256(const unsigned char* data, size_t datalen, const unsigned char* key, size_t keylen, unsigned char* out);

#endif