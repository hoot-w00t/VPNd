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
#include "structs.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <openssl/pem.h>
#include <openssl/err.h>

// load an RSA key in PEM format from a string
// returns NULL on error
RSA *load_rsa_key_from_string(const void *key, uint32_t key_len, bool public)
{
    RSA *rsa_key = NULL;
    BIO *bp = BIO_new_mem_buf(key, key_len);

    if (!bp) {
        logger(LOG_CRIT, "BIO allocation failed\n");
        return NULL;
    }

    if (public) {
        rsa_key = PEM_read_bio_RSA_PUBKEY(bp, &rsa_key, NULL, NULL);
    } else {
        rsa_key = PEM_read_bio_RSAPrivateKey(bp, &rsa_key, NULL, NULL);
    }

    if (!rsa_key) {
        logger(LOG_ERROR, "Could not load %sRSA key from string",
                          public ? "public " : "");
    }
    BIO_free_all(bp);

    return rsa_key;
}

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

static RSA *daemon_privkey = NULL;
static RSA *daemon_pubkey = NULL;

// get daemon private key
RSA *get_daemon_privkey(void)
{
    return daemon_privkey;
}

// get daemon public key
RSA *get_daemon_pubkey(void)
{
    return daemon_pubkey;
}

// load daemon private key
RSA *load_daemon_privkey(void)
{
    char *path = join_paths(daemon_config_dir(), "key.priv");

    if (!path) {
        logger(LOG_CRIT, "Could not allocate memory for daemon privkey path");
        return daemon_privkey;
    }

    RSA_free(daemon_privkey);
    daemon_privkey = NULL;
    daemon_privkey = load_rsa_key(path, false);
    free(path);

    return daemon_privkey;
}

// load daemon public key
RSA *load_daemon_pubkey(void)
{
    char *path = join_paths(daemon_config_dir(), "key.pub");

    if (!path) {
        logger(LOG_CRIT, "Could not allocate memory for daemon pubkey path");
        return daemon_privkey;
    }

    RSA_free(daemon_pubkey);
    daemon_pubkey = NULL;
    daemon_pubkey = load_rsa_key(path, true);
    free(path);

    return daemon_pubkey;
}

// free daemon RSA keys
void free_daemon_keys(void)
{
    RSA_free(daemon_pubkey);
    RSA_free(daemon_privkey);
}

// return true if *k1 and *k2 are the same
bool keys_match(RSA *k1, RSA *k2)
{
    BIO *b1 = BIO_new(BIO_s_mem());
    BIO *b2 = BIO_new(BIO_s_mem());

    if (!b1 || !b2) {
        logger(LOG_CRIT, "Could not allocate memory for keys_match");
        return false;
    }

    PEM_write_bio_RSA_PUBKEY(b1, k1);
    PEM_write_bio_RSA_PUBKEY(b2, k2);

    BUF_MEM *m1 = NULL;
    BUF_MEM *m2 = NULL;

    BIO_get_mem_ptr(b1, &m1);
    BIO_get_mem_ptr(b2, &m2);

    bool match = false;
    if (m1->length == m2->length) {
        if (!strncmp(m1->data, m2->data, m1->length)) {
            match = true;
        }
    }

    BIO_free_all(b1);
    BIO_free_all(b2);
    return match;
}

static struct rsa_key *trusted_keys = NULL;

// add trusted key to the list
void add_trusted_key(RSA *key)
{
    struct rsa_key *rkey = malloc(sizeof(struct rsa_key));

    if (!rkey) {
        logger(LOG_CRIT, "Could not allocate memory for trusted key");
        return;
    }
    rkey->key = key;
    rkey->next = NULL;

    if (!trusted_keys) {
        trusted_keys = rkey;
    } else {
        struct rsa_key *last = trusted_keys;

        while (last->next)
            last = last->next;

        last->next = rkey;
    }
}

// remove all trusted keys
void clear_trusted_keys(void)
{
    struct rsa_key *rkey = trusted_keys;
    struct rsa_key *next = NULL;

    trusted_keys = NULL;
    while (rkey) {
        next = rkey->next;
        RSA_free(rkey->key);
        free(rkey);
        rkey = next;
    }
}

// add all keys in *folder to the trusted keys linked list
void load_trusted_keys(void)
{
    char *path = join_paths(daemon_config_dir(), "trusted_keys");

    if (!path) {
        logger(LOG_CRIT, "Could not allocate memory for trusted_keys path");
        return;
    }

    struct dirent *element = NULL;
    DIR *dirp = opendir(path);

    if (!dirp) {
        logger(LOG_ERROR, "Cannot open trusted keys folder: %s: %s", path, strerror(errno));
        return;
    }

    while ((element = readdir(dirp))) {
        if (element->d_type == DT_REG) {
            char *fullpath = join_paths(path, element->d_name);

            if (!fullpath) {
                logger(LOG_CRIT, "Could not allocate memory for trusted key fullpath");
                closedir(dirp);
                break;
            }

            RSA *key = load_rsa_key(fullpath, true);

            if (key) {
                logger(LOG_DEBUG, "Adding trusted key: %s", element->d_name);
                add_trusted_key(key);
            }

            free(fullpath);
        }
    }

    free(path);
    closedir(dirp);
}

// return true if *key is in the trusted keys linked list
bool is_trusted_key(RSA *key)
{
    struct rsa_key *rkey = trusted_keys;

    while (rkey) {
        if (keys_match(key, rkey->key))
            return true;
        rkey = rkey->next;
    }
    return false;
}