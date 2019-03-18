/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_HW_AUTH_H
#define FPC_TEE_HW_AUTH_H

#include <stdint.h>

#include <errno.h>

#include "fpc_tee.h"

int fpc_tee_set_auth_challenge(fpc_tee_t* tee, uint64_t challenge);
int fpc_tee_get_enrol_challenge(fpc_tee_t* tee, uint64_t* challenge);
int fpc_tee_authorize_enrol(fpc_tee_t* tee, const uint8_t* token,
                            uint32_t size_token);
int fpc_tee_get_auth_result(fpc_tee_t* tee, uint8_t* token,
                            uint32_t size_token);
int fpc_tee_init_hw_auth(fpc_tee_t* tee);

#endif // FPC_TEE_HW_AUTH_H

