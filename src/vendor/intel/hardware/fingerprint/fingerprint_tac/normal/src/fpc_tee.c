/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <limits.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_types.h"
#include "fpc_tac.h"
#include "fpc_ta_targets.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_common_interface.h"
#include "fpc_log.h"

typedef struct fpc_tee fpc_tee_t;

fpc_tee_t* fpc_tee_init()
{
    fpc_tee_t* tee = malloc(sizeof(fpc_tee_t));

    if (!tee) {
        return NULL;
    }

    memset(tee, 0, sizeof(fpc_tee_t));

    tee->tac = fpc_tac_open();
    if (!tee->tac) {
        goto err;
    }

    tee->shared_buffer = fpc_tac_alloc_shared(
        tee->tac,
        MAX_COMMAND_SIZE);

    if (!tee->shared_buffer) {
        goto err;
    }

    return (fpc_tee_t*) tee;

err:
    fpc_tee_release((fpc_tee_t*) tee);
    return NULL;
}

void fpc_tee_release(fpc_tee_t* tee)
{
    if (!tee) {
        return;
    }

    if (tee->shared_buffer) {
        fpc_tac_free_shared(tee->shared_buffer);
    }
    if (tee->tac) {
        fpc_tac_release(tee->tac);
    }

    free(tee);
}

#ifdef FPC_CONFIG_LOGGING_IN_RELEASE_FILE
#ifndef FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE
#define FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE ((uint32_t) 4096)
#endif

#define FPC_CONFIG_LOGGING_IN_RELEASE_FILE_LOCATION "/data/fpc_tpl/error_log"

int fpc_tee_get_error_log(fpc_tee_t *tee)
{
    int status = 0;
    FILE* fd = NULL;

    fpc_tac_shared_mem_t *shared_ipc_buffer =
        fpc_tac_alloc_shared(
            tee->tac,
            FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE + sizeof(fpc_ta_byte_array_msg_t));

    if(!shared_ipc_buffer) {
        status = -FPC_ERROR_ALLOC;
        LOGE("%s: failed to allocate error buffer ret: %d", __func__, status);
        goto out;
    }

    memset(shared_ipc_buffer->addr, 0,
            FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE
            + sizeof(fpc_ta_byte_array_msg_t));

    fpc_ta_common_command_t *command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_COMMON;
    command->header.command = FPC_TA_COMMON_GET_ERROR_LOG_CMD;
    command->error_msg.size = FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }

    status = command->error_msg.response;
    if (status) {
        LOGE("%s, get_error_log ta cmd failed %i.", __func__, status);
        goto out;
    }

    fd = fopen(FPC_CONFIG_LOGGING_IN_RELEASE_FILE_LOCATION, "w");

    if (!fd) {
        status = -errno;
        LOGE("%s, failed to open log file at %s returns error %i",
                __func__,
                FPC_CONFIG_LOGGING_IN_RELEASE_FILE_LOCATION,
                status);

        goto out;
    }

    status = fprintf(fd,"%*s", command->error_msg.size, command->error_msg.array);
    if (status < 0) {
        LOGE("%s, print to log file at %s returns error %i",
                __func__,
                FPC_CONFIG_LOGGING_IN_RELEASE_FILE_LOCATION,
                status);

        status = -FPC_ERROR_IO;
        goto out;
    }
    status = 0;

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }

    if (fd) {
        fclose(fd);
    }
    return status;
}
#else
int fpc_tee_get_error_log(fpc_tee_t *tee) {
    (void) tee;
    return 0;
}

#endif /* FPC_CONFIG_LOGGING_IN_RELEASE_FILE */
