/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>


#include "fpc_types.h"
#include "fpc_log.h"
#include <trusty_std.h>
#include <lib/rng/trusty_rng.h>
#include <string.h>
#include <openssl/hmac.h>


int32_t fpc_secure_random(uint8_t* data, uint32_t length)
{
    trusty_rng_secure_rand(data, length);
    return 0;
}

int fpc_hmac_sha256(const uint8_t* data, uint32_t size_data,
                    const uint8_t* key, uint32_t size_key,
                    uint8_t* hmac)
{
    uint8_t* buf = NULL;
    size_t buf_len = 0;
    buf = HMAC(EVP_sha256(), (void*)key, size_key, data, size_data, hmac, &buf_len);
    if (buf == NULL) {
        LOGE("HMAC() failed");
        return -FPC_ERROR_IO;
    }
    return 0;
}

uint64_t fpc_get_uptime(void)
{
    int64_t time = 0;
    gettime(0, 0, &time);
    return (uint64_t)(time / 1000 / 1000);
}

uint32_t fpc_get_wrapped_size(uint32_t data_size)
{
    return data_size;
}

int32_t fpc_wrap_crypto(uint8_t* data,
                        uint32_t data_size,
                        uint8_t* enc_data,
                        uint32_t* enc_data_size)
{

    LOGI("%s: begin", __func__);
    *enc_data_size = data_size;
    memcpy(enc_data, data, *enc_data_size);

    LOGE("%s: end", __func__);

    return 0;
}

int32_t fpc_unwrap_crypto(uint8_t* enc_data,
                          uint32_t enc_data_size,
                          uint8_t  **data,
                          uint32_t *data_size)
{
    LOGI("%s: begin", __func__);
    *data_size = enc_data_size;

    if(NULL == *data)
        *data = enc_data;
    else
        memcpy(*data, enc_data, *data_size);

    LOGE("%s: end", __func__);

    return 0;
}
