/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_HW_AUTH_H
#define FPC_TA_HW_AUTH_H

#include <stdint.h>

/* 10 minutes */
#define HW_AUTH_CHALLENGE_LIFETIME (1000 * 60 * 10)

/*
 * Data format for an authentication record used to prove successful authentication.
 */
typedef struct __attribute__((__packed__)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;           // in network order
    uint8_t hmac[32];
} fpc_hw_auth_token_t;

int fpc_ta_hw_auth_unwrap_key(uint8_t* encrypted_key,
                              uint32_t size_encrypted_key,
                              uint8_t* key, uint32_t* size_key);

int fpc_ta_hw_has_challenge(void);

int fpc_check_token_integrity(fpc_hw_auth_token_t* token);

#endif //FPC_HW_AUTH_H
