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
#include <openssl/rsa.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef _VPND_RSA
#define _VPND_RSA

#define RSA_PADDING    RSA_PKCS1_OAEP_PADDING
#define RSA_MAXSIZE(k) (RSA_size(k) - 41) // maximum length of data that can be encrypted/decrypted
#define RSA_BUFSIZE(k) (RSA_size(k))      // maximum buffer size to store encrypted/unencrypted data

RSA *load_rsa_key_from_string(const void *key, uint32_t key_len, bool public);
RSA *load_rsa_key(const char *filepath, const bool public);
int rsa_encrypt(byte_t *src, uint32_t src_len, byte_t *dest, RSA *pubkey);
int rsa_decrypt(byte_t *src, uint32_t src_len, byte_t *dest, RSA *privkey);
RSA *get_daemon_privkey(void);
RSA *get_daemon_pubkey(void);
RSA *load_daemon_privkey(void);
RSA *load_daemon_pubkey(void);
void free_daemon_keys(void);
void add_trusted_key(RSA *key);
void clear_trusted_keys(void);
void load_trusted_keys(void);
bool is_trusted_key(RSA *key);

#endif