/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <trusty/tipc.h>
#include <limits.h>
#include <unistd.h>

#include <fpc_tac.h>
#include <fpc_log.h>
#include <fpc_types.h>
#include <fpc_sysfs.h>

struct fpc_tac {
    int st_handle;
    int size_of_max_buffer;
    int sysfs_fd;
};

typedef struct {
    fpc_tac_shared_mem_t shared_mem;
    uint32_t size_mem;
} st_data_t;

#define FPC_TAC_SHARED_BUFFER_SIZE 1024 * 4
#define TRUSTY_DEVICE_NAME "/dev/trusty-ipc-dev0"
#define TADEMO_PORT "com.android.trusty.fpcteeapp"

int trusty_cademo_connect(fpc_tac_t *tac)
{
    int repeat_cnt = 5;
    int rc = 0;

    if (tac->st_handle) {
        LOGE("%s already connected!", __func__);
        return 0;
    }

    do {
        rc = tipc_connect(TRUSTY_DEVICE_NAME, TADEMO_PORT);
        if (rc > 0) {
            break;
        }
    } while (repeat_cnt--);

    if (rc <= 0) {
        LOGE("error, fail to connect tipc port!\n");
        return -FPC_ERROR_IO;
    }

    tac->st_handle = rc;
    return 0;
}

void trusty_cademo_disconnect(fpc_tac_t *tac)
{
    if (tac->st_handle != 0) {
        tipc_close(tac->st_handle);
        tac->st_handle = 0;
    }
}

int trusty_cademo_call(fpc_tac_t *tac, void *in, uint32_t in_size, void *out,
                           uint32_t *out_size)
{
    if (tac->st_handle == 0) {
        LOGE("%s not connected!", __func__);
        return -FPC_ERROR_NOT_INITIALIZED;
    }

    ssize_t rc = write(tac->st_handle, in, in_size);

    if (rc < 0) {
        LOGE("%s failed to send to %s: %s", __func__,
                TADEMO_PORT, strerror(errno));
        return -FPC_ERROR_COMM;
    }

    rc = read(tac->st_handle, out, *out_size);
    if (rc < 0) {
        LOGE("%s failed to retrieve to %s: %s", __func__,
                TADEMO_PORT, strerror(errno));
        return -FPC_ERROR_COMM;
    }

    if ((size_t) rc < in_size) {
        LOGE("invalid response size (%d)\n", (int) rc);
        return -FPC_ERROR_COMM;
    }

    return 0;
}



fpc_tac_t* fpc_tac_open()
{
    fpc_tac_t* tac = malloc(sizeof(fpc_tac_t));
    if (!tac) {
        return NULL;
    }

    memset(tac, 0, sizeof(fpc_tac_t));
    tac->sysfs_fd = -1;

    char path[PATH_MAX];

    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                                path, PATH_MAX)) {
        goto err;
    }

    tac->sysfs_fd = open(path, O_RDONLY);

    if (tac->sysfs_fd == -1) {
        LOGE("%s open %s failed %i", __func__, path, errno);
        goto err;
    }

    tac->size_of_max_buffer = FPC_TAC_SHARED_BUFFER_SIZE;
    return tac;

err:
    if (tac->sysfs_fd != -1) {
        close(tac->sysfs_fd);
    }

    free(tac);
    return NULL;
}

void fpc_tac_release(fpc_tac_t* tac)
{
    if (!tac) {
        return;
    }

    if (tac->sysfs_fd != -1) {
        close(tac->sysfs_fd);
    }

    free(tac);
}


fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t* tac, uint32_t size)
{
    (void)tac; // Unused parameter
    st_data_t* st_data = malloc(sizeof(st_data_t));

    if (!st_data) {
        return NULL;
    }

    st_data->shared_mem.addr = NULL;

    if ((int)size > tac->size_of_max_buffer ) {
        LOGE("%s exceed the max buffer (%d) ", __func__, size);
        goto err;
    }

    st_data->size_mem = size;

    st_data->shared_mem.addr = (void*) malloc (st_data->size_mem);

    if (st_data->shared_mem.addr == NULL) {
        LOGE("%s malloc failed with error %i", __func__, -errno);
        goto err;
    }

    return (fpc_tac_shared_mem_t*) st_data;

err:
    fpc_tac_free_shared((fpc_tac_shared_mem_t*) st_data);
    return NULL;
}

void fpc_tac_free_shared(fpc_tac_shared_mem_t* shared_buffer)
{
    if (!shared_buffer) {
        return;
    }

    st_data_t* st_data = (st_data_t*) shared_buffer;

    if (st_data->shared_mem.addr) {
        free(st_data->shared_mem.addr);
    }

    free(shared_buffer);
}

int fpc_tac_transfer(fpc_tac_t* tac, fpc_tac_shared_mem_t *shared_buffer)
{
    int status = 0;

    st_data_t* recv_buf = NULL;
    st_data_t* send_buf = (st_data_t*) shared_buffer;

    if(!send_buf || !send_buf->shared_mem.addr) {
        LOGE("%s empty send_buff ", __func__);
        return -FPC_ERROR_COMM;
    }

    recv_buf = (st_data_t*) malloc(sizeof(st_data_t));
    if (!recv_buf) {
        LOGE("%s recv_buf alloc failed ", __func__);
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    memset(recv_buf, 0, sizeof(st_data_t));
    recv_buf->size_mem = send_buf->size_mem + sizeof(int32_t);
    recv_buf->shared_mem.addr = (void*) malloc( recv_buf->size_mem);
    if (!recv_buf->shared_mem.addr) {
        LOGE("%s recv_buf->sharee_mem alloc failed ", __func__);
        status = -FPC_ERROR_ALLOC;
        goto out;
    }
    memset(recv_buf->shared_mem.addr, 0, recv_buf->size_mem);

    status = fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "1");
    if (status) {
        goto out;
    }

    status = trusty_cademo_connect(tac);
    if (status) {
        LOGE("%s Error initializing trusty session: %d", __func__, status);
        goto out;
    }

    status = trusty_cademo_call(tac, send_buf->shared_mem.addr, send_buf->size_mem,
                                recv_buf->shared_mem.addr, &recv_buf->size_mem);

    trusty_cademo_disconnect(tac);

    memcpy(shared_buffer->addr, recv_buf->shared_mem.addr, send_buf->size_mem);

    int32_t *response = (int32_t*)((uint8_t*)(recv_buf->shared_mem.addr) + send_buf->size_mem);
    if (status || *response) {
        LOGE("%s send_cmd failed %i", __func__, status ? status : *response);
        status = -FPC_ERROR_COMM;
        goto out;
    }
out:
    fpc_sysfs_node_write(tac->sysfs_fd, "clk_enable", "0");
    if (recv_buf) {
        if(recv_buf->shared_mem.addr) {
            free(recv_buf->shared_mem.addr);
        }
        free(recv_buf);
    }
    return status;
}

