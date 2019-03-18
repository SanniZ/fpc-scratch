/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <string.h>

#include "fpc_ta_targets.h"
#include "fpc_fs.h"
#include "fpc_ta_bio_internal.h"
#include "fpc_ta_bio.h"
#include "fpc_ta_interface.h"
#include "fpc_types.h"
#include "fpc_log.h"
#include "fpc_ta_module.h"


static int fpc_ta_load_db_from_file(fpc_bio_t* bio, const char* path)
{
    LOG_ENTER();
    uint32_t file_size = 0;
    uint8_t* data = NULL;

    fpc_db_destroy(&bio->user_db);

    int status = -fpc_fs_get_filesize(path, &file_size);
    if (status) {
        goto out;
    }

    if (!file_size) {
        status = -FPC_ERROR_NOENTITY;
        goto out;
    }

    data = malloc(file_size);
    if (!data) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    status = fpc_fs_read(path, data, file_size);
    if (status < 0) {
        goto out;
    } else if ((uint32_t)status != file_size) {
        status = -FPC_ERROR_IO;
        goto out;
    }

    status = fpc_db_create(&bio->user_db, data, file_size);
    if (status) {
        goto out;
    }

out:
    free(data);
    return status;
}

static int fpc_ta_store_db_to_file(fpc_bio_t* bio, const char* path)
{
    LOG_ENTER();
    uint8_t* db_data = NULL;

    uint32_t size_db;
    int status = fpc_db_get_data_size(bio->user_db, &size_db);

    if (status) {
        goto out;
    }

    if (size_db == 0) {
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    db_data = malloc(size_db);

    if (!db_data) {
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    status = fpc_db_get_data(bio->user_db, db_data, size_db);
    if (status) {
        goto out;
    }

    status = fpc_fs_write(path, db_data, size_db);
    if (status <= 0) {
        goto out;
    }

    status = 0;

out:
    free(db_data);
    return status;
}

static int fpc_ta_fs_handler(void* buffer, uint32_t size_buffer)
{
    fpc_ta_bio_command_t* command = (fpc_ta_bio_command_t*)buffer;

    fpc_bio_t* bio = fpc_bio_get_handle();

    if (!command) {
        return -FPC_ERROR_INPUT;
    }

    switch (command->header.command) {

    case FPC_TA_BIO_LOAD_DB_CMD:
        if (!byte_array_string_valid(&command->load_db, size_buffer)) {
            return -FPC_ERROR_INPUT;
        }
        command->load_db.response =
                fpc_ta_load_db_from_file(bio, (char*) command->load_db.array);
        break;
    case FPC_TA_BIO_STORE_DB_CMD:
        if (!byte_array_string_valid(&command->store_db, size_buffer)) {
            return -FPC_ERROR_INPUT;
        }
        command->store_db.response =
                fpc_ta_store_db_to_file(bio, (char*) command->store_db.array);
        break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

fpc_ta_module_t fpc_ta_fs_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_ta_fs_handler,
    .key = TARGET_FPC_TA_FS,
};
