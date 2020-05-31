#include "encryption.h"
#include "vpnd.h"
#include "logger.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// key == 256 bits == 32 bytes
// iv == 128 bits == 16 bytes

int aes_encrypt(byte_t *data, uint32_t data_len, byte_t *key,
    byte_t *iv, byte_t *enc_data)
{
    EVP_CIPHER_CTX *ctx;
    int len, enc_data_len;

    ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        logger(LOG_CRIT, "Could not allocate memory for encryption");
        return -1;
    }

    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        logger(LOG_ERROR, "Encryption initialization failed"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (EVP_EncryptUpdate(ctx, enc_data, &len, data, data_len) != 1) {
        logger(LOG_ERROR, "Encryption failed at EVP_EncryptUpdate"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    enc_data_len = len;

    if(EVP_EncryptFinal_ex(ctx, enc_data + len, &len) != 1) {
        logger(LOG_ERROR, "Encryption failed at EVP_EncryptFinal_ex"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    enc_data_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return enc_data_len;
}

int aes_decrypt(byte_t *enc_data, uint32_t enc_data_len, byte_t *key,
    byte_t *iv, byte_t *data)
{
    EVP_CIPHER_CTX *ctx;
    int len, data_len;

    ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        logger(LOG_CRIT, "Could not allocate memory for decryption");
        return -1;
    }

    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        logger(LOG_ERROR, "Decryption initialization failed"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if(1 != EVP_DecryptUpdate(ctx, data, &len, enc_data, enc_data_len)) {
        logger(LOG_ERROR, "Decryption failed at EVP_DecryptUpdate"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    data_len = len;

    if(EVP_DecryptFinal_ex(ctx, data + len, &len) != 1) {
        logger(LOG_ERROR, "Decryption failed at EVP_DecryptFinal_ex"); // add error messages
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    data_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return data_len;
}