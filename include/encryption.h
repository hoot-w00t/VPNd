#include "vpnd.h"
#include <stdint.h>

#ifndef _VPND_ENCRYPTION
#define _VPND_ENCRYPTION

int aes_encrypt(byte_t *data, uint32_t data_len, byte_t *key,
    byte_t *iv, byte_t *enc_data);
int aes_decrypt(byte_t *enc_data, uint32_t enc_data_len, byte_t *key,
    byte_t *iv, byte_t *data);

#endif