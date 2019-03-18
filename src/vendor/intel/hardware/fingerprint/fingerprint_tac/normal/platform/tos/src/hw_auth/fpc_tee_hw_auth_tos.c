/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#include <hardware/hw_auth_token.h>

#include "fpc_ta_hw_auth_interface.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_targets.h"
#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_types.h"

int fpc_tee_init_hw_auth(fpc_tee_t* tee)
{
    int status = 0;
    const uint32_t size = sizeof(fpc_ta_hw_auth_command_t);

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    fpc_ta_hw_auth_command_t* command = shared_buffer->addr;
    command->header.command = FPC_TA_HW_AUTH_SET_SHARED_KEY;
    command->header.target = TARGET_FPC_TA_HW_AUTH;
    command->set_shared_key.size = 0;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto free;
    }

    status = command->set_shared_key.response;

free:
    fpc_tac_free_shared(shared_buffer);
out:
    return status;
}
