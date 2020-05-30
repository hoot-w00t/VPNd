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

#include "rsa.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/pem.h>
#include <openssl/err.h>

// load an RSA key in PEM format
// returns NULL on error
RSA *load_rsa_key(const char *filepath, const bool public)
{
    RSA *key = NULL;
    FILE *fp = fopen(filepath, "r");

    if (!fp) {
        logger(LOG_ERROR, "Could not open file: %s: %s",
                          filepath,
                          strerror(errno));
        return NULL;
    }

    if (public) {
        key = PEM_read_RSA_PUBKEY(fp, &key, NULL, NULL);
    } else {
        key = PEM_read_RSAPrivateKey(fp, &key, NULL, NULL);
    }

    if (!key) {
        logger(LOG_ERROR, "Could not load %sRSA key: %s",
                          public ? "public " : "",
                          filepath);
    }

    fclose(fp);
    return key;
}

// encrypt *src into *dest using *pubkey
int rsa_encrypt(byte_t *src, uint32_t src_len, byte_t *dest, RSA *pubkey)
{
    if (!pubkey)
        return -1;

    if (src_len > (unsigned) RSA_MAXSIZE(pubkey)) {
        logger(LOG_CRIT, "rsa_encrypt: src_len is too big, cannot encrypt");
        return -1;
    }

    int enc_len = RSA_public_encrypt(src_len, src, dest, pubkey, RSA_PADDING);
    if (enc_len < 0) {
        logger(LOG_CRIT, "rsa_encrypt error: %s",
                         ERR_error_string(ERR_get_error(), NULL));
    }

    return enc_len;
}

// decrypt *src into *dest using *privkey
int rsa_decrypt(byte_t *src, uint32_t src_len, byte_t *dest, RSA *privkey)
{
    if (!privkey)
        return -1;

    if (src_len > (unsigned) RSA_size(privkey)) {
        logger(LOG_CRIT, "rsa_decrypt: src_len is too big, cannot decrypt");
        return -1;
    }

    int dec_len = RSA_private_decrypt(src_len, src, dest, privkey, RSA_PADDING);
    if (dec_len < 0) {
        logger(LOG_CRIT, "rsa_decrypt error: %s",
                         ERR_error_string(ERR_get_error(), NULL));
    }

    return dec_len;
}