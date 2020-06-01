#include "vpnd.h"
#include <stdint.h>
#include <stdbool.h>
#include <openssl/conf.h>
#include <openssl/evp.h>

#ifndef _VPND_ENCRYPTION
#define _VPND_ENCRYPTION

EVP_CIPHER_CTX *aes_init_ctx(byte_t *key, byte_t *iv, bool encrypt);
int aes_encrypt(EVP_CIPHER_CTX *ctx, byte_t *data, uint32_t data_len,
    byte_t *enc_data);
int aes_decrypt(EVP_CIPHER_CTX *ctx, byte_t *enc_data, uint32_t enc_data_len,
    byte_t *data);

#endif