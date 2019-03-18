/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fpc_irq_device.h"
#include "fpc_log.h"
#include "fpc_types.h"

#include "fpc_sysfs.h"

struct fpc_irq {
    int sysfs_fd;
    int cancel_fds[2];
};

void fpc_irq_release(fpc_irq_t* device)
{
    if (!device) {
        return;
    }

    if (device->sysfs_fd != -1) {
        close(device->sysfs_fd);
    }

    if (device->cancel_fds[0] != -1) {
        close(device->cancel_fds[0]);
    }

    if (device->cancel_fds[1] != -1) {
        close(device->cancel_fds[1]);
    }

    free(device);
}

fpc_irq_t* fpc_irq_init()
{
    fpc_irq_t* device = malloc(sizeof(fpc_irq_t));

    if (!device) {
        goto err;
    }

    device->sysfs_fd = -1;
    device->cancel_fds[0] = -1;
    device->cancel_fds[1] = -1;

    char path[PATH_MAX];
    if (!fpc_sysfs_path_by_attr(FPC_REE_DEVICE_ALIAS_FILE, FPC_REE_DEVICE_NAME, FPC_REE_DEVICE_PATH,
                                path, PATH_MAX)) {
        LOGE("%s Error didn't find phys path device", __func__);
        goto err;
    }

    device->sysfs_fd = open(path, O_RDONLY);

    if (device->sysfs_fd == -1) {
        LOGE("%s open %s failed %i", __func__, path, errno);
        goto err;
    }

    if (pipe(device->cancel_fds)) {
        goto err;
    }

    return device;

err:

    fpc_irq_release(device);
    return NULL;
}

int fpc_irq_wait(fpc_irq_t* device, int irq_value)
{
    int irq_fd = -1;
    int status = 0;

    for(;;) {
        irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
        if (irq_fd == -1) {
            LOGE("%s openat failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        }

        char value = 0;

        status = read(irq_fd, &value, sizeof(value));

        if (status < 0) {
            LOGE("%s read failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (status == 0) {
            status = -FPC_ERROR_IO;
            goto out;
        }

        if ((value - '0') == irq_value) {
            status = 0;
            goto out;
        }

        struct pollfd pfd[2];
        pfd[0].fd = irq_fd;
        pfd[0].events = POLLERR | POLLPRI;
        pfd[1].fd = device->cancel_fds[0];
        pfd[1].events = POLLIN;

        status = poll(pfd, 2, -1);

        /* acknowledge that poll returned, can be used to
         * measure latency from the kernel driver. */
        fpc_sysfs_node_write(device->sysfs_fd, "irq", "1");

        if (status == -1) {
            LOGE("%s poll failed with error %i", __func__, -errno);
            status = -FPC_ERROR_IO;
            goto out;
        } else if (pfd[1].revents) {
            status = -FPC_ERROR_CANCELLED;
            goto out;
        }
        close(irq_fd);
    }
out:
    if (irq_fd != -1) {
        close(irq_fd);
    }

    return status;
}

int fpc_irq_set_cancel(fpc_irq_t* device)
{
    int status = 0;
    uint8_t byte = 1;
    if (write(device->cancel_fds[1], &byte, sizeof(byte)) != sizeof(byte)) {
        LOGE("%s write failed %i", __func__, errno);
        status = -FPC_ERROR_IO;
    }

    return status;
}

int fpc_irq_clear_cancel(fpc_irq_t* device)
{
    int status = 0;
    uint8_t byte;
    if (read(device->cancel_fds[0], &byte, sizeof(byte)) < 0) {
        LOGE("%s read failed %i", __func__, errno);
        status = -FPC_ERROR_IO;
    }

    return status;
}

int fpc_irq_status(fpc_irq_t* device)
{
    int irq_fd = -1;
    int status = 0;
    char value = 0;

    irq_fd = openat(device->sysfs_fd, "irq", O_RDONLY | O_NONBLOCK);
    if (irq_fd == -1) {
        LOGE("%s openat failed with error %s", __func__, strerror(-errno));
        status = -FPC_ERROR_IO;
        goto out;
    }

    // Value will be the IRQ pin value, status will be the amount of bytes read
    status = read(irq_fd, &value, sizeof(value));
    if (status < 0) {
        LOGE("%s read failed with error %s", __func__, strerror(-errno));
        status = -FPC_ERROR_IO;
        goto out;
    } else if (status == 0) {
        LOGE("%s no bytes available to read from irq node", __func__);
        status = -FPC_ERROR_IO;
        goto out;
    }

    status = value - '0';
    if (status < 0 || status > 9) {
        LOGE("%s got a strange value from irq node: %d", __func__, status);
        status = -FPC_ERROR_IO;
    }

out:
    if (irq_fd != -1) {
        close(irq_fd);
    }

    return status;
}

int fpc_irq_wakeup_enable(fpc_irq_t *device)
{
    return fpc_sysfs_node_write(device->sysfs_fd, "wakeup_enable", "enable");
}

int fpc_irq_wakeup_disable(fpc_irq_t *device)
{
    return fpc_sysfs_node_write(device->sysfs_fd, "wakeup_enable", "disable");
}
