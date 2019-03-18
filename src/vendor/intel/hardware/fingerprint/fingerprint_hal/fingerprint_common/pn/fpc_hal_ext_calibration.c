/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "fpc_hal_ext_calibration.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "container_of.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_pn.h"
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <fcntl.h>
#include "container_of.h"
#include "fpc_tee.h"
#include "fpc_log.h"
#include "fpc_hal_pn.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_hal_ext_calibration_service.h"
#include "fpc_hal_private.h"

#define PN_CALIBRATION_MAX_TRIES            3
#define PN_CALIBRATION_START_SLEEP_TIME_MS  200
#define PN_CALIBRATION_RETRY_SLEEP_TIME_MS  100
#define USECS_PER_MSEC                      1000

typedef struct {
    fpc_calibration_t calibration;
    pthread_mutex_t mutex;
    void* status_cb_ctx;
    fpc_calibration_status_cb_t status_cb;
    void* error_cb_ctx;
    fpc_calibration_error_cb_t error_cb;
    fpc_hal_common_t* hal;
} calibration_module_t;

static int cb_calibration_status(fpc_calibration_t* self, uint8_t code)
{
    LOGD("%s\n", __func__);
    int status = 0;
    calibration_module_t* module = (calibration_module_t*) self;

    if (!module->status_cb) {
        LOGD("%s no callback registered\n", __func__);
        status = -1;
        goto out;
    }

    status = module->status_cb(module->status_cb_ctx, code);

out:

    return status;
}

static int cb_calibration_error(fpc_calibration_t* self, int8_t error)
{
    LOGD("%s\n", __func__);
    int status = 0;
    calibration_module_t* module = (calibration_module_t*) self;

    if (!module->error_cb) {
        LOGD("%s no callback registered\n", __func__);
        status = -1;
        goto out;
    }

    status = module->error_cb(module->error_cb_ctx, error);

out:

    return status;
}

static int wait_for_sensor_input(fpc_hal_common_t* hal) {
    int status;

    do {
        status = fpc_tee_wait_finger_lost(hal->sensor);
    } while (status == FPC_CAPTURE_FINGER_STUCK);


    if (status) {
        return status;
    }

    status = fpc_tee_wait_finger_down(hal->sensor);
    if (status) {
        return status;
    }

    return 0;
}

static void do_calibrate_pn(void* data)
{
    LOGD("%s", __func__);

    int status = 0;
    uint32_t image_size;
    void *image = NULL;
    calibration_module_t* module = (calibration_module_t*) data;
    struct timespec tp;
    uint64_t start_time_ms;
    uint64_t end_time_ms;

    pthread_mutex_lock(&module->mutex);

    fpc_hal_common_t* hal = module->hal;

    LOGD("%s, waiting for sensor input", __func__);
    cb_calibration_status(hal->ext_calibration,
                FPC_PN_CALIBRATION_STATUS_WAITING_FOR_INPUT);
    status = wait_for_sensor_input(hal);

    if (status) {
        goto out;
    }

    (void)clock_gettime(CLOCK_MONOTONIC, &tp);
    start_time_ms = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

    LOGD("%s, getting size", __func__);
    status = fpc_tee_pn_get_size(hal->sensor, &image_size);

    if (status) {
        goto out;
    }

    image = malloc(sizeof(uint8_t) * image_size);

    LOGD("%s, stabilizing", __func__);
    cb_calibration_status(hal->ext_calibration, FPC_PN_CALIBRATION_STATUS_STABILIZE);

    usleep(PN_CALIBRATION_START_SLEEP_TIME_MS * USECS_PER_MSEC);

    LOGD("%s, starting", __func__);
    cb_calibration_status(hal->ext_calibration, FPC_PN_CALIBRATION_STATUS_START);

    for (int i = 0; i < PN_CALIBRATION_MAX_TRIES; i++) {
        status = fpc_tee_pn_calibrate(hal->sensor, image, image_size);

        if (status == -FPC_PN_RETRY_CALIBRATION) {
            LOGD("%s, sensor not fully covered, retrying...", __func__);
            cb_calibration_status(hal->ext_calibration, FPC_PN_CALIBRATION_STATUS_RETRY);
            usleep(PN_CALIBRATION_START_SLEEP_TIME_MS * USECS_PER_MSEC);
        } else if (status) {
            goto out;
        } else {
            break;
        }
    }

    if (status) {
        goto out;
    }

    LOGD("%s, saving image", __func__);
    status = fpc_save_pn(image, image_size);

    if (status) {
        goto out;
    }

    status = fpc_load_pn(hal);

    if (status) {
        goto out;
    }

#ifdef FPC_CONFIG_ENGINEERING
    fpc_save_pn_debug(hal);
#endif

    LOGD("%s, done", __func__);

    cb_calibration_status(hal->ext_calibration, FPC_PN_CALIBRATION_STATUS_DONE);

    (void)clock_gettime(CLOCK_MONOTONIC, &tp);
    end_time_ms = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    LOGD("%s: KPI pn calibrate duration %" PRIu64 " ms", __func__, (end_time_ms - start_time_ms));

out:
    free(image);
    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        cb_calibration_error(hal->ext_calibration, status);
    }

    pthread_mutex_unlock(&module->mutex);
}

static int calibrate_pn(fpc_calibration_t* self)
{
    LOGD("%s", __func__);

    int status = 0;
    calibration_module_t* module = (calibration_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    fingerprint_hal_do_async_work(module->hal, do_calibrate_pn, module, FPC_TASK_HAL_EXT);

    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static void set_calibration_cb(fpc_calibration_t* self,
        fpc_calibration_status_cb_t status_callback,
        fpc_calibration_error_cb_t error_callback, void *ctx)
{
    LOGD("%s\n", __func__);

    calibration_module_t* module = (calibration_module_t*) self;

    pthread_mutex_lock(&module->mutex);

    module->status_cb = status_callback;
    module->status_cb_ctx = ctx;
    module->error_cb = error_callback;
    module->error_cb_ctx = ctx;

    pthread_mutex_unlock(&module->mutex);
}

fpc_calibration_t* fpc_calibration_new(fpc_hal_common_t* hal)
{
    LOGD("%s\n", __func__);

    calibration_module_t* module = malloc(sizeof(calibration_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(calibration_module_t));
    module->hal = hal;
    module->calibration.set_calibration_cb = set_calibration_cb;
    module->calibration.calibrate_pn = calibrate_pn;

    pthread_mutex_init(&module->mutex, NULL);

    return (fpc_calibration_t*) module;
}

void fpc_calibration_destroy(fpc_calibration_t* self)
{
    if (!self) {
        return;
    }

    calibration_module_t* module = (calibration_module_t*) self;
    pthread_mutex_destroy(&module->mutex);
    free(self);
}
