/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdint.h>
#include <trusty_ipc.h>
#include <lib/keymaster/keymaster.h>
#include "fpc_ta_hw_auth_interface.h"
#include "fpc_ta_hw_auth.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_std_libs.h"



int fpc_ta_hw_auth_unwrap_key(uint8_t* encrypted_key,
                              uint32_t size_encrypted_key,
                              uint8_t* key, uint32_t* size_key)
{
    (void)encrypted_key;
    (void)size_encrypted_key;
    uint8_t* key_buf_p = NULL;

    long rc = keymaster_open();
    if (rc < 0) {
        return -1;
    }

    keymaster_session_t session = (keymaster_session_t) rc;

    rc = keymaster_get_auth_token_key(session, &key_buf_p, size_key);
    if (rc < 0) {
        LOGE("fp get hmac key has failed with code: %d", rc);
        rc = -1;
        goto out;
    } else {
        LOGD("fp get hmac key success");
        memcpy(key, key_buf_p, *size_key);
        rc = 0;
    }
out:
    keymaster_close(session);
    return rc;
}
