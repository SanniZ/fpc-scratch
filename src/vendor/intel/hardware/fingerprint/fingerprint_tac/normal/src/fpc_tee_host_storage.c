/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tac.h"
#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))

static int get_file_size(const char* path, size_t* size)
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

static int fpc_read_db_blob_from_file(void *buf, int size, const char* path)
{
    mode_t mode = S_IRUSR | S_IWUSR;
    int c = 0;
    int fd;

    fd = open(path, O_RDONLY, mode);

    if (fd < 0) {
        LOGE("%s failed to open file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    }

    c = read(fd, buf, size);
    close(fd);

    if (size != c) {
        LOGE("%s failed to read full size of file %s ", __func__, path);
        return -FPC_ERROR_IO;
    } else {
        LOGD("%s Successfully read template database %s", __func__, path);
        return 0;
    }
}

static int fpc_write_db_blob_to_file(void *buf, int c, const char* path)
{
    mode_t mode = S_IRUSR | S_IWUSR;
    int r = 0;
    int fd;

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        LOGE("%s failed to open file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    }

    r = write(fd, buf, c);
    fsync(fd);
    close(fd);

    if (r == -1) {
        LOGE("%s failed to write file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    } else if (r != c) {
        LOGE("%s failed to write full size of file %s", __func__, path);
        return -FPC_ERROR_IO;
    }

    LOGD("%s Successfully wrote template database %s", __func__, path);

    return 0;
}

static int get_db_blob_size(fpc_tee_t* tee, size_t *blob_size)
{
    int status = 0;

    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;
    command->header.target    = TARGET_FPC_TA_DB_BLOB;
    command->header.command   = FPC_TA_BIO_GET_DB_SIZE_CMD;
    command->get_db_size.size = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    *blob_size = command->get_db_size.size;

    return (status < 0) ? status : command->get_db_size.response;
}

static int db_open(fpc_tee_t *tee, uint32_t mode, uint32_t size)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_OPEN_CMD;
    command->db_open.mode   = mode;
    command->db_open.size   = size;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (result < 0) {
        LOGE("%s Failed to complete command", __func__);
    } else {
        result = command->db_open.bio.response;
    }

    return result;
}

static int db_close(fpc_tee_t *tee)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_CLOSE_CMD;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (result < 0) {
        LOGE("%s Failed to complete command", __func__);
    } else {
        result = command->db_close.response;
        if (result < 0) {
            LOGE("%s TA returned error %d", __func__, result);
        }
    }

    return result;
}

static int send_db_read_commands(fpc_tee_t* tee, uint8_t *blob, size_t blob_size)
{
    int result = 0;
    const uint32_t payload_size = MIN(blob_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_READ_CMD;
    command->db_read.size         = payload_size;

    uint32_t blob_offset    = 0;
    uint32_t remaining_size = blob_size;

    // Send read commands until complete payload is transfered
    while (remaining_size) {
        uint32_t payload_size = MIN(MAX_CHUNK, remaining_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (result < 0) {
            goto free_shared;
        }

        result = command->db_read.response;
        if (result < 0) {
            goto free_shared;
        }

        remaining_size -= payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -FPC_ERROR_COMM;
            goto free_shared;
        }

        memcpy(&blob[blob_offset], command->db_read.array, payload_size);
        blob_offset += payload_size;
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

static int send_db_write_commands(fpc_tee_t* tee, uint8_t *file_buffer, size_t file_buffer_size)
{
    int result = 0;
    fpc_tac_shared_mem_t* shared_buffer = NULL;

    // Setup write message
    const uint32_t payload_size = MIN(file_buffer_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_WRITE_CMD;
    command->db_write.size        = payload_size;

    uint32_t remaining_size = file_buffer_size;
    uint32_t payload_offset = 0;

    // Send write commands until complete payload is transfered
    while (remaining_size) {
        uint32_t payload_size = MIN(MAX_CHUNK, remaining_size);
        memcpy(command->db_write.array, &file_buffer[payload_offset], payload_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (result < 0) {
            LOGE("%s - Failed to write %u bytes of data. Result %d",__func__, payload_size, result);
            goto free_shared;
        }

        result = command->db_write.response;
        if (result < 0) {
            LOGE("%s - Failed to write %u bytes of data. TA responded %d", __func__, payload_size, result);
            goto free_shared;
        }
        remaining_size -= payload_size;
        payload_offset += payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -1;
            goto free_shared;
        }
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

int fpc_tee_store_template_db(fpc_tee_bio_t* bio, const char* path)
{
    fpc_tee_t* tee = &bio->tee;
    char temp_path[PATH_MAX];
    int result;
    size_t blob_size = 0;

    if (snprintf(temp_path, PATH_MAX, "%s.bak", path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        return -FPC_ERROR_INPUT;
    }

    result = get_db_blob_size(tee, &blob_size);
    if (result < 0) {
        return result;
    } else if (blob_size == 0) {
        return -FPC_ERROR_IO;
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_RDONLY, blob_size);
    if (result < 0) {
        LOGE("%s - transfer_open failed with %d", __func__, result);
        goto close;
    }

    uint8_t *blob = malloc(blob_size);
    if (!blob) {
        result = -FPC_ERROR_ALLOC;
        goto close;
    }

    // Transfer encrypted content from TA (chunk if necessary)
    result = send_db_read_commands(tee, blob, blob_size);
    if (result < 0) {
        goto free;
    }

    result = fpc_write_db_blob_to_file(blob, blob_size, temp_path);
    if (result < 0) {
        LOGE("%s - Failed (%d) to write template db to file system. Keeping the old database.", __func__, result);
        goto free;
    }

    if (rename(temp_path, path) < 0) {
        LOGE("%s - rename failed with error %s", __func__, strerror(errno));
        (void)unlink(temp_path);
        result = -FPC_ERROR_IO;
        goto free;
    }

free:
    free(blob);
close:
    db_close(tee);
    return result;
}

int fpc_tee_load_template_db(fpc_tee_bio_t* bio, const char* path)
{
    fpc_tee_t* tee = &bio->tee;

    size_t file_size = 0;

    int result = get_file_size(path, &file_size);
    if (result < 0) {
        return result;
    }

    if (file_size == 0) {
        return fpc_tee_load_empty_db(bio);
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_WRONLY, file_size);
    if (result < 0) {
        LOGE("Failed to open transfer in write mode with %zu bytes of payload", file_size);
        goto close;
    }

    uint8_t *file_buffer = malloc(file_size);
    if (!file_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    result = fpc_read_db_blob_from_file(file_buffer, file_size, path);
    if (result < 0) {
        goto free;
    }

    // Transfer encrypted content to TA (chunk if necessary)
    result = send_db_write_commands(tee, file_buffer, file_size);
    if (result < 0) {
        goto free;
    }

free:
    free(file_buffer);
close:
    db_close(tee);
    return result;
}
