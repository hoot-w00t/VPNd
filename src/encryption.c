#include "encryption.h"
#include "vpnd.h"
#include "logger.h"
#include <openssl/err.h>

// key == 256 bits == 32 bytes
// iv == 128 bits == 16 bytes
// initialize AES-256-CBC cipher
// returns NULL on error
EVP_CIPHER_CTX *aes_init_ctx(byte_t *key, byte_t *iv, bool encrypt)
{
    EVP_CIPHER_CTX *ctx;

    ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        logger(LOG_CRIT, "Could not allocate memory for encryption");
        return NULL;
    }

    if (encrypt) {
        if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
            logger(LOG_ERROR, "Init EVP_EncryptInit_ex failed: %s",
                              ERR_error_string(ERR_get_error(), NULL));
            EVP_CIPHER_CTX_free(ctx);
            return NULL;
        }
    } else {
        if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
            logger(LOG_ERROR, "Init EVP_DecryptInit_ex failed: %s",
                              ERR_error_string(ERR_get_error(), NULL));
            EVP_CIPHER_CTX_free(ctx);
            return NULL;
        }
    }

    return ctx;
}

int aes_encrypt(EVP_CIPHER_CTX *ctx, byte_t *data, uint32_t data_len,
    byte_t *enc_data)
{
    int len, enc_data_len;

    if(EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL) != 1) {
        logger(LOG_ERROR, "EVP_EncryptInit_ex failed: %s",
                           ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    if (EVP_EncryptUpdate(ctx, enc_data, &len, data, data_len) != 1) {
        logger(LOG_ERROR, "EVP_EncryptUpdate failed: %s",
                          ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }
    enc_data_len = len;

    if(EVP_EncryptFinal_ex(ctx, enc_data + len, &len) != 1) {
        logger(LOG_ERROR, "EVP_EncryptFinal_ex failed: %s",
                          ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    return enc_data_len + len;
}

int aes_decrypt(EVP_CIPHER_CTX *ctx, byte_t *enc_data, uint32_t enc_data_len,
    byte_t *data)
{
    int len, data_len;

    if(EVP_DecryptInit_ex(ctx, NULL, NULL, NULL, NULL) != 1) {
        logger(LOG_ERROR, "EVP_DecryptInit_ex failed: %s",
                          ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    if(1 != EVP_DecryptUpdate(ctx, data, &len, enc_data, enc_data_len)) {
        logger(LOG_ERROR, "EVP_DecryptUpdate failed: %s",
                          ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }
    data_len = len;

    if(EVP_DecryptFinal_ex(ctx, data + len, &len) != 1) {
        logger(LOG_ERROR, "EVP_DecryptFinal_ex failed: %s",
                          ERR_error_string(ERR_get_error(), NULL));
        return -1;
    }

    return data_len + len;
}