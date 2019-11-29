/*
   PBKDF2 (Password Based Key Derivation Function)
   This function derives a good cryptographic key from
   a possibly weak password given by a human user.
   
   https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-132.pdf
   
   Parameters:
   
      password - The plain password typed in by the user (length-terminated).
      salt     - Cryptographic salt - a random sequence of bytes (length-terminated).
      key      - Pointer to at least keylen bytes to output the generated key.
      keylen   - Length of the key to generate in bytes.
      rounds   - Number of rounds
      hmac     - Keyed-hash message authentication code
      diglen   - Output length of hmac in bytes
   
*/
#ifndef __PBKDF_H
#define __PBKDF_H
#include <stdlib.h>

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
);

#endif