/*
   AES-256 IN COUNTER MODE
   These are the AES-265 encryption and decryption functions used with counter mode.
   
   Parameters:
   
      in   - Input data for encryption (length-terminated)
      key  - Key used for encryption
      iv   - Initialization vector
      out  - Buffer for encrypted data that is at least as large as in (can be in)
   
*/
#ifndef __AES_CTR_256_H
#define __AES_CTR_256_H
#include <stddef.h>

void aes_ctr_256(const unsigned char* in, size_t inlen, const unsigned char key[32], const unsigned char iv[12], unsigned char* out);

#endif