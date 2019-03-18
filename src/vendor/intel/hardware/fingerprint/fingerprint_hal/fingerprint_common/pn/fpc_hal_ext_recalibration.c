/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
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
#include "fpc_hal_ext_recalibration.h"
#include "fpc_hal_pn.h"
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
#include "fpc_tee.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_hal_ext_recalibration_service.h"
#include "fpc_worker.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_hal_private.h"

#define USECS_PER_MSEC                      1000

typedef struct {
    fpc_recalibration_t recalibration;
    pthread_mutex_t mutex;
    void* status_cb_ctx;
    fpc_recalibration_status_cb_t status_cb;
    void* error_cb_ctx;
    fpc_recalibration_error_cb_t error_cb;
    fpc_hal_common_t* hal;
    uint8_t* token;
    uint32_t size_token;
    int canceled;
} recalibration_module_t;

static void cb_recalibration_status(fpc_recalibration_t* self, int32_t code,
        int32_t image_decision, int32_t image_quality, int32_t pn_quality,
        int32_t progress)
{
    LOGD("%s\n", __func__);
    recalibration_module_t* module = (recalibration_module_t*) self;

    if (!module->status_cb) {
        LOGD("%s no callback registered\n", __func__);
        return;
    }

    module->status_cb(module->status_cb_ctx, code, image_decision, image_quality, pn_quality, progress);
}

static void cb_recalibration_error(fpc_recalibration_t* self, int32_t error)
{
    LOGD("%s\n", __func__);
    recalibration_module_t* module = (recalibration_module_t*) self;

    if (!module->error_cb) {
        LOGD("%s no callback registered\n", __func__);
        return;
    }

    module->error_cb(module->error_cb_ctx, error);

}

static void reset_recalibration(recalibration_module_t* module)
{
    module->error_cb = NULL;
    module->error_cb_ctx = NULL;
    module->status_cb = NULL;
    module->status_cb_ctx = NULL;
    free(module->token);
    module->token = NULL;
    module->size_token = 0;
}

static int wait_for_sensor_input(fpc_hal_common_t* hal) {
    int status;

    do {
        status = fpc_tee_wait_finger_lost(hal->sensor);
    } while (status == FPC_CAPTURE_FINGER_STUCK);

    status = fpc_tee_wait_finger_down(hal->sensor);
    if (status) {
        return status;
    }

    return 0;
}

static void do_recalibrate_pn(void* data)
{
    LOGD("%s", __func__);
    int status = 0;

    recalibration_module_t* module = (recalibration_module_t*) data;
    fpc_hal_common_t* hal = module->hal;

    int32_t error_code = 0;
    int32_t image_decision = 0;
    int32_t image_quality = 0;
    int32_t pn_quality = 0;
    int32_t progress = 0;
    void *pn_buffer = NULL;
    uint32_t pn_size;
    int cancel = 0;

#ifdef FPC_CONFIG_HW_AUTH
    status = fpc_tee_pn_authorize(hal->sensor, module->token, module->size_token);
    if (status) {
        LOGD("%s, authorize failed", __func__);
        error_code = FPC_PN_RECALIBRATION_ERROR_AUTHORIZATION;
        goto out;
    }
#endif

    cb_recalibration_status(hal->ext_recalibration, FPC_PN_RECALIBRATION_STATUS_WAITING_FOR_INPUT, 0, 0, 0, 0);

    status = wait_for_sensor_input(hal);

    if (status) {
        error_code = FPC_PN_RECALIBRATION_ERROR_CANCELED;
        goto out;
    }

    do {
        status = fpc_tee_pn_calibrate_finger(hal->sensor, &image_decision,
                &image_quality, &pn_quality, &progress);

        pthread_mutex_lock(&module->mutex);
        cancel = module->canceled;
        pthread_mutex_unlock(&module->mutex);

        if (status == -FPC_PN_FAILED || status == -FPC_ERROR_CANCELLED || cancel) {
            //cleanup and exit
            fpc_tee_pn_calibrate_finger_end(hal->sensor, NULL, 0);
            error_code = cancel ? FPC_PN_RECALIBRATION_ERROR_CANCELED : FPC_PN_RECALIBRATION_ERROR_FAILED;
            goto out;
        }

        cb_recalibration_status(hal->ext_recalibration,
                FPC_PN_RECALIBRATION_STATUS_UPDATE, image_decision,
                image_quality, pn_quality, progress);

    } while (status == -FPC_PN_RETRY_CALIBRATION);

    if (status != FPC_PN_OK) {
        LOGD("%s calibration failed with status %d", __func__, status);
        error_code = FPC_PN_RECALIBRATION_ERROR_INTERNAL;
        fpc_tee_pn_calibrate_finger_end(hal->sensor, NULL, 0);
        goto out;
    }

    LOGD("%s calibration complete, saving file.", __func__);

    status = fpc_tee_pn_get_size(hal->sensor, &pn_size);

    if (status) {
        error_code = FPC_PN_RECALIBRATION_ERROR_INTERNAL;
        goto out;
    }

    LOGD("%s size: %i", __func__, pn_size);

    pn_buffer = malloc(sizeof(uint8_t) * pn_size);

    if (pn_buffer == NULL) {
        LOGD("%s, No memory for pn_buffer", __func__);
        status = -FPC_ERROR_ALLOC;
        error_code = FPC_PN_RECALIBRATION_ERROR_MEMORY;
        goto out;
    }

    status = fpc_tee_pn_calibrate_finger_end(hal->sensor, pn_buffer, pn_size);

    if (status) {
        error_code = FPC_PN_RECALIBRATION_ERROR_INTERNAL;
        goto out;
    }

    LOGD("%s, saving image", __func__);
    status = fpc_save_pn(pn_buffer, pn_size);

    if (status) {
        error_code = FPC_PN_RECALIBRATION_ERROR_INTERNAL;
        goto out;
    }

    status = fpc_load_pn(hal);

    if (status) {
        error_code = FPC_PN_RECALIBRATION_ERROR_INTERNAL;
        goto out;
    }

#ifdef FPC_CONFIG_ENGINEERING
    fpc_save_pn_debug(hal);
#endif

    LOGD("%s, done", __func__);

    cb_recalibration_status(hal->ext_recalibration,
            FPC_PN_RECALIBRATION_STATUS_DONE, 0, 0, 0, 0);

out:
    if(status) {
        LOGE("%s failed %i\n", __func__, status);
        cb_recalibration_error(hal->ext_recalibration, error_code);
    }

    free(pn_buffer);
    reset_recalibration(module);
}

static int pre_recalibrate_pn(fpc_recalibration_t* self, uint64_t* challenge)
{
    LOGD("%s", __func__);

    int status = 0;

    recalibration_module_t* module = (recalibration_module_t*) self;

#ifdef FPC_CONFIG_HW_AUTH
    status = fpc_tee_pn_get_challenge(module->hal->sensor, challenge);
    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        *challenge = 0;
    }
#endif

    return status;
}

static int recalibrate_pn(fpc_recalibration_t* self, const uint8_t* token, ssize_t token_length)
{
    LOGD("%s", __func__);

    int status = 0;
    recalibration_module_t* module = (recalibration_module_t*) self;

    module->canceled = 0;
    module->token = malloc(sizeof(uint8_t) * token_length);

    if (module->token == NULL) {
        LOGD("%s, No memory for token", __func__);
        return -FPC_ERROR_ALLOC;
    }

    memcpy(module->token, token, token_length);
    module->size_token = token_length;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_do_async_work(module->hal, do_recalibrate_pn, (void*) module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static int cancel_recalibration(fpc_recalibration_t* self) {
    LOGD("%s", __func__);

    int status = 0;
    recalibration_module_t* module = (recalibration_module_t*) self;

    pthread_mutex_lock(&module->mutex);
    module->canceled = 1;
    pthread_mutex_unlock(&module->mutex);

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    reset_recalibration(module);

    return status;
}

static void set_recalibration_cb(fpc_recalibration_t* self,
        fpc_recalibration_status_cb_t status_callback,
        fpc_recalibration_error_cb_t error_callback, void *ctx)
{
    LOGD("%s\n", __func__);

    recalibration_module_t* module = (recalibration_module_t*) self;

    pthread_mutex_lock(&module->mutex);

    module->status_cb = status_callback;
    module->status_cb_ctx = ctx;
    module->error_cb = error_callback;
    module->error_cb_ctx = ctx;

    pthread_mutex_unlock(&module->mutex);
}

fpc_recalibration_t* fpc_recalibration_new(fpc_hal_common_t* hal)
{
    LOGD("%s\n", __func__);

    recalibration_module_t* module = malloc(sizeof(recalibration_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(recalibration_module_t));
    module->hal = hal;
    module->recalibration.set_recalibration_cb = set_recalibration_cb;
    module->recalibration.recalibrate_pn = recalibrate_pn;
    module->recalibration.pre_recalibrate_pn = pre_recalibrate_pn;
    module->recalibration.cancel_recalibration = cancel_recalibration;

    pthread_mutex_init(&module->mutex, NULL);

    return (fpc_recalibration_t*) module;
}

void fpc_recalibration_destroy(fpc_recalibration_t* self)
{
    if (!self) {
        return;
    }

    recalibration_module_t* module = (recalibration_module_t*) self;
    pthread_mutex_destroy(&module->mutex);
    free(self);
}
