/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdio.h>
#include <stdlib.h>
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_crypto.h"
#include "lib/storage/storage.h"

#define FS_FTS_PORT STORAGE_CLIENT_TD_PORT

/******************************************************************************/
int fpc_fs_remove(const char* path)
{
    int32_t ret = 0;
    storage_session_t fss;
    ret = storage_open_session(&fss, FS_FTS_PORT);
    if (ret < 0) {
        LOGE("failed (%d) to open session\n", ret);
        return -FPC_ERROR_NOENTITY;
    }

    ret = storage_delete_file(fss, path, STORAGE_OP_COMPLETE);
    if (0 != ret) {
        LOGE("%s: storage_delete_file failed for %s with error %d",
             __func__, path, ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }
out:
    storage_end_transaction(fss, true);
    storage_close_session(fss);

    return ret;
}

/******************************************************************************/
int fpc_fs_read(const char* path, uint8_t *buf, uint32_t size)
{
    int read_bytes = 0;
    int32_t crypto_ret = 0;
    uint32_t unwrapped_size = 0;
    uint8_t *data = NULL;
    storage_session_t fss;
    file_handle_t handle = 0;
    storage_off_t offset = 0;
    int ret = 0;

    LOGD("%s: Start",__func__);
    if (NULL == buf) {
        LOGE("%s: Read buffer is NULL", __func__);
        return -FPC_ERROR_INPUT;
    }

    data = malloc(size);
    if (NULL == data) {
        LOGE("%s: Failed to allocate memory for data", __func__);
        return -FPC_ERROR_ALLOC;
    }

    ret = storage_open_session(&fss, FS_FTS_PORT);
    if (ret < 0) {
        LOGE("failed (%d) to open session", ret);
        ret = -FPC_ERROR_NOENTITY;
        goto out;
    }

    // open file  STORAGE_FILE_OPEN_CREATE
    ret = storage_open_file(fss, &handle, path, STORAGE_FILE_OPEN_CREATE, 0);
    if (ret < 0) {
        LOGE("failed (%d) to open file", ret);
        ret = -FPC_ERROR_NOENTITY;
        goto out;
    }

    read_bytes =  (int)storage_read(handle, offset, (void*)data, (size_t) size);
    if (0 > read_bytes) {
        LOGE("%s: storage_read failed for %s with error %d",
                __func__, path, read_bytes);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    crypto_ret = fpc_unwrap_crypto(data, read_bytes, &buf, &unwrapped_size);
    if (0 > crypto_ret) {
        LOGE("%s: fpc_unwrap_crypto failed with error %d",
                __func__, crypto_ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    LOGD("%s :  Read file %d Bytes , Read actual data: %d",
            __func__, read_bytes, unwrapped_size);

out:
    if(data) {
        free(data);
    }
    if (handle) {
        storage_close_file(handle);
    }
    storage_end_transaction(fss, true);
    storage_close_session(fss);
    if (ret == 0) {
        ret = read_bytes;
    }
    return ret;
}

/******************************************************************************/
int fpc_fs_write(const char* path, uint8_t *buf, uint32_t size)
{
    int written_bytes = 0;
    int32_t crypto_ret = 0;
    uint8_t *data;
    uint32_t data_size = 0;
    storage_session_t fss;
    file_handle_t handle = 0;
    storage_off_t offset = 0;
    int ret = 0;

    LOGD( "%s Writing file %s Start Using FTS", __func__, path);
    if (NULL == buf) {
        LOGE("%s: Write buffer is NULL", __func__);
        return -FPC_ERROR_INPUT;
    }

    data_size = fpc_get_wrapped_size(size);
    data = malloc(data_size);
    if (NULL == data) {
        LOGE("%s: Failed to allocate memory for data", __func__);
        return -FPC_ERROR_ALLOC;
    }

    crypto_ret = fpc_wrap_crypto(buf, size, data, &data_size);
    if (0 > crypto_ret) {
        LOGE("%s: fpc_wrap_crypto failed with error %d",
                __func__, crypto_ret);
        ret = -FPC_ERROR_IO;
        goto out;
    }

    LOGD("%s: storage_write for %s size: %d",
            __func__, path, data_size);

    ret = storage_open_session(&fss, FS_FTS_PORT);
    if (ret < 0) {
        LOGE("failed (%d) to open session", ret);
        ret = -FPC_ERROR_NOENTITY;
        goto out;
    }

    // open file  STORAGE_FILE_OPEN_CREATE
    ret = storage_open_file(fss, &handle, path, STORAGE_FILE_OPEN_CREATE, 0);
    if (ret < 0) {
        LOGE("failed (%d) to open file", ret);
        ret = -FPC_ERROR_NOENTITY;
        goto out;
    }

    written_bytes =  (int)storage_write(handle, offset, (void*)data, (size_t) size, STORAGE_OP_COMPLETE);
    if (0 > written_bytes) {
        LOGE("%s: storage_write failed for %s with error %d",
                __func__, path, written_bytes);
        ret = -FPC_ERROR_IO;
        goto out;
    }
    else if(data_size != (uint32_t)written_bytes) {
        LOGE("%s: requested size :%d for writing %s"
              "  does not match written bytes %d ",
                __func__,
                data_size,
                path,
                written_bytes);

        ret = -FPC_ERROR_IO;
        goto out;
    }

    LOGD("%s FTS  ret: %d written_bytes: %d bytes",
            __func__, ret, written_bytes);

out:
    if(data) {
        free(data);
    }
    if (handle) {
        storage_close_file(handle);
    }
    storage_end_transaction(fss, true);
    storage_close_session(fss);
    if (ret == 0) {
        ret = written_bytes;
    }

    return ret;
}

/******************************************************************************/
int fpc_fs_get_filesize(const char* path, uint32_t *size)
{
    storage_off_t retval = 0;
    storage_session_t fss;
    file_handle_t handle = 0;
    int ret = 0;

    ret = storage_open_session(&fss, FS_FTS_PORT);
    if (ret < 0) {
        LOGE("failed (%d) to open session", ret);
        ret = FPC_ERROR_NOENTITY;
        goto out;
    }

    // open file  STORAGE_FILE_OPEN_CREATE
    ret = storage_open_file(fss, &handle, path, STORAGE_FILE_OPEN_CREATE, 0);
    if (ret < 0) {
        LOGE("failed (%d) to open file", ret);
        ret = FPC_ERROR_NOENTITY;
        goto out;
    }

    ret = storage_get_file_size(handle, &retval);
    if(0 > ret) {
        LOGE("%s:failed to get the file size: %s ret: %d",__func__,path, ret);
        ret = FPC_ERROR_IO;
        goto out;
    }

    *size = (uint32_t)retval;
    LOGD("%s: file size: %s : %d", __func__, path, *size);
out:
    if (handle) {
        storage_close_file(handle);
    }
    storage_end_transaction(fss, true);
    storage_close_session(fss);
    return ret;
}
