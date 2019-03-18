/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tac.h"
#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"

static int get_file_size(const char* path, uint32_t* size)
{
    struct stat file_info;

    memset(&file_info, 0, sizeof(file_info));
    *size = 0;

    if (stat(path, &file_info)) {
        switch(errno) {
        case ENOENT:
            return 0;
        default:
            LOGE("%s stat failed with error %s", __func__, strerror(errno));
            return -FPC_ERROR_IO;
        }
    }

    *size = file_info.st_size;

    return 0;
}

static int db_command(fpc_tee_bio_t* bio, int32_t op, const char* path)
{
    fpc_tee_t* tee = &bio->tee;
    uint32_t size_string = strlen(path) + 1;
    uint32_t size = sizeof(fpc_ta_byte_array_msg_t) + size_string;

    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, size);

    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    int status;
    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.command = op;
    command->header.target = TARGET_FPC_TA_FS;
    command->store_db.size = size_string;
    memcpy(command->store_db.array, path, size_string);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->store_db.response;

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_store_template_db(fpc_tee_bio_t* bio, const char* path)
{
    LOGD("%s", __func__);
    char temp_path[PATH_MAX];
    int status;

    if (snprintf(temp_path, PATH_MAX, "%s.bak", path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        return -FPC_ERROR_INPUT;
    }

    status = db_command(bio, FPC_TA_BIO_STORE_DB_CMD, temp_path);
    if (status) {
        goto out;
    }

    if (rename(temp_path, path)) {
        LOGE("%s stat failed with error %s", __func__, strerror(errno));
        status = -FPC_ERROR_IO;
    }

out:
    remove(temp_path);
    return status;
}

int fpc_tee_load_template_db(fpc_tee_bio_t* bio, const char* path)
{
    LOGD("%s", __func__);
    uint32_t size;
    int status = get_file_size(path, &size);
    if (status) {
        return status;
    }

    if (size == 0)
    {
        return fpc_tee_load_empty_db(bio);
    }

    status = db_command(bio, FPC_TA_BIO_LOAD_DB_CMD, path);
    if(-FPC_ERROR_IO == status)
    {
        //-FPC_ERROR_IO is returned on file read and crypto errors.
        //In both cases we assume that the db is non-recoverable.
        LOGE("%s, failed to load db, corrupt?, load empty db.", __func__);
        status = fpc_tee_load_empty_db(bio);
    }

    return status;
}
