/*
    VPNd - a VPN daemon
    Copyright (C) 2020  akrocynova

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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