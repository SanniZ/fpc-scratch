/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef INCLUSION_GUARD_FPC_CRYPTO
#define INCLUSION_GUARD_FPC_CRYPTO

#include <stdint.h>
#include <stddef.h>

/**
 * Compare memory areas mem_1 and mem_2 at a maximum nbr_bytes number of bytes are compared.
 *
 * @param[in]  mem_1      Pointer to memory area 1
 * @param[in]  mem_2      Pointer to memory area 2
 * @param[in]  nbr_bytes  Maximum number of bytes to be compared
 *
 * @return 0 if nbr_bytes number of bytes of memory areas mem_1 and mem_2 are equal
 *         non-zero otherwise.
 */
int fpc_crypto_memcmp(const uint8_t *mem_1,
                      const uint8_t *mem_2,
                      size_t nbr_bytes);

/**
 *
 * @return uptime in milliseconds
 */
uint64_t fpc_get_uptime(void);

/**
 * Get random numbers from a secure random number generator
 *
 * @param[in]  data           array of bytes to be overwritten with random data
 * @param[in]  data_size      no of bytes of random data to be writen
 *
 * @return     the the number of bytes written
 */
int32_t fpc_secure_random(uint8_t* data, uint32_t data_size);

/**
 * compute hmac using sha256 algorithm and given key
 *
 * @param data input data
 * @param size_data size of data
 * @param key crypto key
 * @param size_key size of the key
 * @param 256bit hmac output, buffer allocated by caller.
 * @return 0 on success
 */
int fpc_hmac_sha256(const uint8_t* data, uint32_t size_data,
                    const uint8_t* key, uint32_t size_key,
                    uint8_t* hmac);

/**
  * get size of encrypted object from size of data.
  *
  * @param data_size size of buffer to encrypt.
  *
  * @return size encrypted object after wrapping.
  */
uint32_t fpc_get_wrapped_size(uint32_t data_size);

/**
 * wrap buffer in encryption object
 *
 * @param data buffer to wrap
 * @param data_size size of data in bytes.
 * @param pointer to buffer for encrypted data.
 * @param in: size of buffer for encrypted data
 *        out: enc_data_size size of decrypted data.
 *
 * @return 0 on success, FPC_ERROR_IO on crypto failure.
 */
int32_t fpc_wrap_crypto(uint8_t* data,
                    uint32_t data_size,
                    uint8_t* enc_data,
                    uint32_t* enc_data_size);

/**
 * unwrap encryption object
 *
 * @param enc_data encrypted object
 * @param enc_data_size size of encrypted object in bytes.
 * @param pointer to pointer of buffer for unencrypted data.
 *        If NULL, data will be decrypted inplace in enc_data
 *        and data will point to start of decrypted data.
 * @param data_size size of decrypted data.
 *
 * @return 0 on success, FPC_ERROR_IO on crypto failure.
 */
int32_t fpc_unwrap_crypto(uint8_t* enc_data,
                    uint32_t enc_data_size,
                    uint8_t  **data,
                    uint32_t *data_size);

#endif //INCLUSION_GUARD_FPC_CRYPTO

