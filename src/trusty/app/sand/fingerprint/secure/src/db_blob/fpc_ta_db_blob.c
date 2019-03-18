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

#include "fpc_ta_bio_internal.h"
#include "fpc_ta_bio.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_ta_interface.h"
#include "fpc_db.h"
#include "fpc_types.h"
#include "fpc_log.h"
#include "fpc_crypto.h"
#include "fpc_ta_module.h"
#include "fpc_ta_targets.h"

#ifndef MIN
#define MIN(A, B) ( (A) < (B) ? (A) : (B) )
#endif

static int fpc_ta_get_db_blob_size(fpc_bio_t* bio, uint32_t* blob_size)
{
    LOGD("%s", __func__);
    uint32_t data_size;

    int status = fpc_db_get_data_size(bio->user_db, &data_size);
    if (status) {
        return status;
    }

    *blob_size = fpc_get_wrapped_size(data_size);

    return 0;
}

static struct
{
    int32_t  mode;        // O_RDONLY, O_WRONLY
    uint32_t size;        // transfer size
    uint32_t remaining;   // remaining bytes
    uint32_t data_offset; // offset during assembly
    uint8_t *data;        // data
} transfer =
{
    .mode        = -1,
    .size        =  0,
    .remaining   =  0,
    .data_offset =  0,
    .data        =  NULL
};

static int fpc_ta_db_close(void)
{
    free(transfer.data);
    memset(&transfer, 0, sizeof(transfer));
    transfer.mode = -1;
    return 0;
}

static int unwrap(fpc_bio_t* bio)
{
    int result = 0;

    uint32_t size_data = transfer.size;
    uint8_t *data      = NULL;

    result = fpc_unwrap_crypto(transfer.data, transfer.size, &data, &size_data);
    if (result < 0) {
        LOGE("%s failed. Result %d", __func__, result);
        goto out;
    }

    result = fpc_db_create(&bio->user_db, data, size_data);
    if (result < 0) {
        LOGE("%s failed. Result %d", __func__, result);
        goto out;
    }
out:
    return result;
}

// TODO:  We need to limit the allocation size. The actual max size is currently not
//        available from algo (ALGO-5640). Should be ~200 KB per template, use 250 KB.
#define FPC_TA_MAX_TRANSFER (MAX_NR_TEMPLATES * 250 * 1024)

// REE opens a transfer towards TEE
// After a successful 'db open' the state variable 'transfer' will be
// setup and ready to be used for subsequent read|write operations
static int fpc_ta_db_open(fpc_bio_t* bio, uint32_t mode, uint32_t size)
{
    int result = 0;
    uint8_t* unwrapped_data = NULL;

    if (transfer.mode != -1) {
        return -FPC_ERROR_IO;
    }

    if ((size == 0) || (size > FPC_TA_MAX_TRANSFER)) {
        LOGE("%s - invalid size %u", __func__, size);
        return -FPC_ERROR_INPUT;
    }

    switch (mode) {

        // Write mode implies REE will write encrypted database to TEE.
        case FPC_TA_BIO_DB_WRONLY: {
            transfer.data = malloc(size);
            if (!transfer.data) {
                LOGE("%s Allocation (1) failed", __func__);
                return -FPC_ERROR_ALLOC;
            }
            transfer.size        = size;
            transfer.remaining   = size;
            transfer.mode        = FPC_TA_BIO_DB_WRONLY;
            transfer.data_offset = 0;
        }
        break;

        // Read mode implies REE request the encrypted database to be transfered from TEE
        case FPC_TA_BIO_DB_RDONLY: {
            uint32_t unwrapped_data_size = 0;
            transfer.mode = FPC_TA_BIO_DB_RDONLY;

            result = fpc_db_get_data_size(bio->user_db, &unwrapped_data_size);
            if ((result < 0) || (unwrapped_data_size == 0)) {
                LOGE("%s:%d Result %d. Size %u", __func__, __LINE__, result, size);
                result = -FPC_ERROR_IO;
                goto out;
            }

            // Allocate memory for unwrapped data
            unwrapped_data = malloc(unwrapped_data_size);
            if (!unwrapped_data) {
                LOGE("%s Allocation (2) failed", __func__);
                result = -FPC_ERROR_ALLOC;
                goto out;
            }

            // Allocate memory for the wrapped data
            transfer.size        = fpc_get_wrapped_size(unwrapped_data_size);
            transfer.data_offset = 0;
            transfer.data        = malloc(transfer.size);

            if (!transfer.data) {
                LOGE("%s Allocation (3) failed", __func__);
                result = -FPC_ERROR_ALLOC;
                goto out;
            }

            result = fpc_db_get_data(bio->user_db, unwrapped_data, unwrapped_data_size);
            if (result < 0) {
                LOGE("%s:%d Result %d.", __func__, __LINE__, result);
                result = -FPC_ERROR_IO;
                goto out;
            }

            uint32_t wrapped_size = transfer.size;

            result = fpc_wrap_crypto(
                        unwrapped_data,      // [IN] source buffer
                        unwrapped_data_size, // [IN] source data size
                        transfer.data,       // [IN] destination buffer
                        &wrapped_size);      // [IN] destination buffer size [OUT] wrapped data size

            if (result < 0) {
                LOGE("%s:%d Result %d.", __func__, __LINE__, result);
                goto out;
            }

            if (wrapped_size != transfer.size) {
                LOGE("%s:%d wrapped size %u and allocated size %u differs",
                     __func__, __LINE__, wrapped_size, transfer.size);
                result = -FPC_ERROR_IO;
                goto out;
            }
            transfer.remaining = transfer.size;
        }
        break;

        default: {
            LOGE("%s Invalid mode %u.", __func__, mode);
            result = -FPC_ERROR_INPUT;
        }
    }

out:
    if (unwrapped_data) {
        free(unwrapped_data);
        unwrapped_data = NULL;
    }

    if (result < 0) {
        fpc_ta_db_close();
    }
    return result;
}

// REE writes data to TEE. Only valid in case db was opened in mode O_WRONLY
static int fpc_ta_db_write(fpc_bio_t* bio, uint8_t* buffer, uint32_t size)
{
    LOGD("%s - %u bytes remaining. Got %u bytes.", __func__, transfer.remaining, size);

    if (transfer.mode != FPC_TA_BIO_DB_WRONLY) {
        return -FPC_ERROR_INPUT;
    }

    if ((size == 0) || (transfer.remaining == 0)) {
        return -FPC_ERROR_INPUT;
    }

    size = MIN(transfer.remaining, size);
    uint8_t* dest = transfer.data + transfer.data_offset;

    memcpy(dest, buffer, size);

    transfer.remaining   -= size;
    transfer.data_offset += size;

    if (transfer.remaining == 0) {
        return unwrap(bio);
    }
    return transfer.remaining;
}

// REE reads data from TEE. Only valid in case db was opened in mode O_RDONLY
static int fpc_ta_db_read(uint8_t* buffer, uint32_t size) {

    if ((size == 0) || (transfer.mode != FPC_TA_BIO_DB_RDONLY) || (transfer.remaining == 0)) {
        return -FPC_ERROR_INPUT;
    }

    uint8_t* src = transfer.data + transfer.data_offset;
    size = MIN(transfer.remaining, size);

    LOGD("%s - Copy %u bytes (%u bytes remaining)", __func__, size, transfer.remaining);
    memcpy(buffer, src, size);
    transfer.remaining   -= size;
    transfer.data_offset += size;

    return transfer.remaining;
}

static int fpc_ta_db_blob_handler(void* buffer, uint32_t size_buffer)
{
    fpc_ta_bio_command_t* command = shared_cast_to(
            fpc_ta_bio_command_t,
            buffer,
            size_buffer);

    if (!command) {
        return -FPC_ERROR_INPUT;
    }

    fpc_bio_t* bio = fpc_bio_get_handle();

    switch (command->header.command) {
    case FPC_TA_BIO_GET_DB_SIZE_CMD:
        command->get_db_size.response =
                fpc_ta_get_db_blob_size(bio, &command->get_db_size.size);
        break;

    case FPC_TA_BIO_DB_OPEN_CMD:
        command->db_open.bio.response = fpc_ta_db_open(
                bio,
                command->db_open.mode,
                command->db_open.size);
        break;

    case FPC_TA_BIO_DB_CLOSE_CMD:
        command->db_close.response = fpc_ta_db_close();
        break;

    case FPC_TA_BIO_DB_READ_CMD:
        if (!byte_array_valid(&command->db_read, size_buffer)) {
            return -FPC_ERROR_INPUT;
        }
        command->db_read.response = fpc_ta_db_read(
                command->db_read.array,
                command->db_read.size);
        break;

    case FPC_TA_BIO_DB_WRITE_CMD:
        if (!byte_array_valid(&command->db_write, size_buffer)) {
            return -FPC_ERROR_INPUT;
        }
        command->db_write.response = fpc_ta_db_write(
                bio,
                command->db_write.array,
                command->db_write.size);
        break;

    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

fpc_ta_module_t fpc_ta_db_blob_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_ta_db_blob_handler,
    .key = TARGET_FPC_TA_DB_BLOB,
};
