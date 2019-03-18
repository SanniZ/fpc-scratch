/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_BIO_INTERNAL_H
#define FPC_TEE_BIO_INTERNAL_H

#include "fpc_tee_bio.h"
#include "fpc_tee_internal.h"

#define _get_bio_cmd_struct(bio) \
    ((fpc_ta_bio_command_t*) bio->tee.shared_buffer->addr);

struct fpc_tee_bio {
    fpc_tee_t tee;
};

#endif // FPC_TEE_BIO_INTERNAL_H

