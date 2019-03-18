/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "container_of.h"
#include "fpc_hal_pn.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_tee_pn.h"
#include "fpc_hal_private.h"

static int fpc_hal_fs_get_filesize(const char* path, uint32_t *size)
{
    int retval = 0;
    int fd = -1;

    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        LOGE("%s: failed to open the file : %s with error: %s",
            __func__,
            path,
            strerror(errno));

        return -FPC_ERROR_IO;
    }

    retval = lseek(fd, 0, SEEK_END);
    if(-1 == retval) {
        LOGE("%s:failed to get the file size: %s with error: %s",
            __func__,
            path,
            strerror(errno));

        close(fd);
        return -FPC_ERROR_IO;
    }

    *size = retval;

    retval = close(fd);
    if (-1 == retval) {
        LOGE("%s:failed to close the file size: %s with error: %s",
            __func__,
            path,
            strerror(errno));

        return -FPC_ERROR_IO;
    }

    return retval;
}

static int fpc_hal_fs_read(const char* path, uint8_t *buf, uint32_t size)
{
    ssize_t read_bytes = 0;
    int fd = 0;

    LOGD("%s: Start",__func__);
    if (NULL == buf) {
        LOGE("%s: Read buffer is NULL", __func__);
        read_bytes = -FPC_ERROR_INPUT;
        goto out;
    }
    fd = open(path,O_RDONLY, S_IRUSR);
    if(fd < 0) {
        LOGE("%s: open() failed for %s with error %s",
                __func__, path, strerror(errno));

        read_bytes = -FPC_ERROR_IO;
        goto out;
    }

    read_bytes = read(fd, buf, size);
    if(read_bytes < 0) {
        LOGE("%s read() failed for %s with error %s",
              __func__, path, strerror(errno));

        read_bytes = -FPC_ERROR_IO;
        goto out;
    }
out:
    close(fd);

    return read_bytes;
}

static int fpc_hal_fs_write(const char* path, uint8_t *buf, uint32_t size)
{
    ssize_t written_bytes = 0;
    int fd;
    if (NULL == buf) {
        LOGE("%s: Write buffer is NULL", __func__);
        return -FPC_ERROR_INPUT;
    }

    fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if(fd < 0) {
        LOGE("%s: open() failed for %s with error %s",
                __func__, path, strerror(errno));

        written_bytes = -FPC_ERROR_IO;
        goto out;
    }

    written_bytes = write(fd, buf, size);
    if(written_bytes < 0) {
        LOGE("%s write() failed for %s with error %s",
              __func__, path, strerror(errno));

        written_bytes = -FPC_ERROR_IO;
        goto out;
    }


out:
    close(fd);
    LOGD("%s: Wrote %d bytes", __func__, (int32_t) written_bytes);

    return written_bytes;
}

#ifdef FPC_CONFIG_ENGINEERING

int fpc_save_pn_debug(fpc_hal_common_t* hal)
{
    LOGD("%s", __func__);

    int status = 0;
    uint32_t image_size;
    void *image = NULL;
    int written_bytes = 0;

    status = fpc_tee_pn_get_unencrypted_size(hal->sensor, &image_size);

    if (status) {
        goto out;
    }

    image = malloc(sizeof(uint8_t) * image_size);

    if (!image) {
        LOGE("%s: failed to allocate buffer", __func__);
        goto out;
    }

    status = fpc_tee_pn_get_unencrypted_image(hal->sensor, image, image_size);

    if (status) {
        goto out;
    }

    LOGD("%s writing\n", __func__);

    written_bytes = fpc_hal_fs_write(PN_CALIBRATION_DEBUG_PATH, image,
            image_size);

    if (written_bytes <= 0) {
        LOGE("%s write failed\n", __func__);
        status = -1;
    }

out:
    if (status) {
        LOGE("%s failed %i\n", __func__, status);
    }

    free(image);

    return status;
}

#endif
int fpc_save_pn(void *image, uint32_t image_size)
{
    LOGD("%s", __func__);

    int status = 0;
    int written_bytes = 0;

    written_bytes = fpc_hal_fs_write(PN_CALIBRATION_PATH, image, image_size);

    if (written_bytes <= 0) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        status = -1;
    }

    return status;
}

int fpc_load_pn(fpc_hal_common_t* dev)
{
    LOGD("%s", __func__);

    int status = 0;
    int read_bytes = 0;

    uint8_t* image = NULL;
    uint32_t image_size;

    char* path = PN_CALIBRATION_PATH;

    if (access(path, F_OK) != -1) {
        status = fpc_hal_fs_get_filesize(path, &image_size);

        if (status) {
            goto out;
        }

        image = malloc(image_size * sizeof(uint8_t));

        read_bytes = fpc_hal_fs_read(path, image, image_size);

        if (read_bytes <= 0) {
            status = -1;
            goto out;
        }

        LOGD("%s loading complete \n", __func__);
        status = fpc_tee_pn_load(dev->sensor, image, image_size);

    } else {
        LOGE("%s no calibration data found\n", __func__);
        goto out;
    }

out:

    if (status) {
        LOGE("%s failed with error %d \n", __func__, status);
    }

    free(image);
    return status;
}
