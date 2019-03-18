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

#include "fpc_hal_ext_engineering.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "container_of.h"
#include "fpc_tee_bio.h"
#include "fpc_worker.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_hal.h"
#include "fpc_hal_private.h"

#define TEST_TEMPLATE_DB "/data/fpc/test.db"

typedef struct {
    fpc_engineering_t engineering;

    pthread_mutex_t mutex;
    void* img_subscr_cb_ctx;
    fpc_img_subscr_cb_t img_subscr_cb;
    void* img_inj_cb_ctx;
    fpc_img_inj_cb_t img_inj_cb;
    bool is_inj_wait_for_img;
    bool is_inj_cancel;
    fpc_img_inj_cancel_cb_t img_inj_cancel_cb;
    uint8_t sensor_width;
    uint8_t sensor_height;
    fpc_capture_cb_t capture_cb;
    void* capture_cb_ctx;
    fpc_hal_common_t* hal;
} engineering_module_t;

int fpc_hal_ext_get_raw_image(fpc_engineering_t* self, fpc_hal_img_data_t* image_data)
{
    LOGD("%s\n", __func__);

    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    status = fpc_tee_debug_get_raw_size(module->hal->tee_handle,
            &image_data->buffer_size);

    if (status) {
        LOGE("%s: fpc_tee_debug_get_raw_size failed with status %d", __func__,
                status);
        image_data->buffer_size = 0;
        return status;
    }

    image_data->image_type = RAW;

    image_data->buffer = (uint8_t*) malloc(image_data->buffer_size);

    if (!image_data->buffer) {
        status = -ENOMEM;
        LOGE("%s: Failed to allocate memory for subscription data with status %d", __func__, status);
        goto out;
    }

    status = fpc_tee_debug_retrieve(module->hal->tee_handle,
                                    FPC_TEE_ENGINEERING_TYPE_RAW,
                                    image_data->buffer,
                                    image_data->buffer_size);

    if (status) {
        LOGE("%s: fpc_tee_debug_retrieve failed with status %d", __func__,
                status);
        goto out;
    }

out:

    return status;
}

int fpc_hal_ext_get_enhanced_image(fpc_engineering_t* self, fpc_hal_img_data_t* image_data)
{
    LOGD("%s\n", __func__);

    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    image_data->buffer_size = module->sensor_width * module->sensor_height;
    image_data->buffer = (uint8_t*) malloc(image_data->buffer_size);

    if (!image_data->buffer) {
        status = -ENOMEM;
        LOGE(
                "%s: Failed to allocate memory for image data with status %d",
                __func__, status);
        goto out;
    }

    status = fpc_tee_debug_retrieve(module->hal->tee_handle,
            FPC_TEE_ENGINEERING_TYPE_ENHANCED_IMAGE,
            image_data->buffer,
            image_data->buffer_size);

    if (status) {
        LOGE("%s: fpc_tee_debug_retrieve failed with status %d", __func__,
                status);
        goto out;
    }

    image_data->image_type = PREPROCESSED;

out:

    return status;
}

void fpc_hal_ext_free_image(fpc_hal_img_data_t* image_data)
{
    free(image_data->buffer);
    image_data->buffer = NULL;
    image_data->buffer_size = 0;
}

static void free_captured_images(fpc_capture_data_t* capture_data)
{
    fpc_hal_ext_free_image(&capture_data->raw_image);
    fpc_hal_ext_free_image(&capture_data->enhanced_image);
}

static int handle_subscription_callback(fpc_engineering_t* self, fpc_capture_data_t* data)
{
    LOGD("%s\n", __func__);
    engineering_module_t* module = (engineering_module_t*) self;

    int status = 0;

    pthread_mutex_lock(&module->mutex);

    status = fpc_hal_ext_get_raw_image(self, &data->raw_image);

    if (status) {
        LOGE("%s: fpc_hal_ext_get_raw_image failed with status %d", __func__, status);
        goto out;
    }

    status = fpc_hal_ext_get_enhanced_image(self, &data->enhanced_image);

    if (status) {
        LOGE("%s: fpc_get_enhanced_image failed with status %d", __func__, status);
        goto out;
    }

    module->img_subscr_cb(module->img_subscr_cb_ctx, data);

out:

    free_captured_images(data);

    pthread_mutex_unlock(&module->mutex);
    return status;
}

static int handle_image_subscription(fpc_engineering_t* self)
{
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    if (!module->img_subscr_cb) {
        LOGD("%s No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_CAPTURE;

    status = handle_subscription_callback(self, &capture_data);

out:

    return status;
}

static int handle_image_subscription_enroll(fpc_engineering_t* self, int capture_result,
        int enroll_result, int samples_remaining, uint32_t fid)
{
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    if (!module->img_subscr_cb) {
        LOGD("%s No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_ENROLL;
    capture_data.user_id = fid;
    capture_data.capture_result = capture_result;
    capture_data.enroll_result = enroll_result;
    capture_data.samples_remaining = samples_remaining;

    status = handle_subscription_callback(self, &capture_data);

out:

    return status;
}

static int handle_image_subscription_auth(fpc_engineering_t* self, int capture_result,
        int identify_result, int32_t coverage, int32_t quality, uint32_t fid)
{
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    if (!module->img_subscr_cb) {
        LOGD("%s No image subscription callback registered\n", __func__);
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_VERIFY;
    capture_data.user_id = fid;
    capture_data.coverage = coverage;
    capture_data.quality = quality;
    capture_data.capture_result = capture_result;
    capture_data.identify_result = identify_result;

    status = handle_subscription_callback(self, &capture_data);

out:

    return status;
}

static void get_sensor_size(fpc_engineering_t* self,
                                  uint8_t* width, uint8_t* height)
{
    LOGD("%s\n", __func__);
    engineering_module_t* module = (engineering_module_t*) self;

    pthread_mutex_lock(&module->mutex);

    *width = module->sensor_width;
    *height = module->sensor_height;

    pthread_mutex_unlock(&module->mutex);
}

static int set_img_subscr_cb(fpc_engineering_t* self,
                             fpc_img_subscr_cb_t callback, void *ctx)
{
    LOGD("%s\n", __func__);

    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;
    pthread_mutex_lock(&module->mutex);

    if (!callback) {
        // NULL pointer callback means we deregister image subscription and ignore ctx parameter
        ctx = NULL;
    }

    module->img_subscr_cb = callback;
    module->img_subscr_cb_ctx = ctx;

    pthread_mutex_unlock(&module->mutex);

    return status;
}

static int save_test_db(engineering_module_t* module)
{
    return fpc_tee_store_template_db(module->hal->bio, TEST_TEMPLATE_DB);
}

static int delete_test_db(engineering_module_t* module)
{
    int status = fpc_tee_load_empty_db(module->hal->bio);
    if (status) {
        LOGE("%s fpc_tee_load_empty_db failed %d", __func__, status);
        return status;
    }
    status = save_test_db(module);
    if (status) {
        LOGE("%s save_test_db failed %d", __func__, status);
    }
    return status;
}

static int load_test_db(engineering_module_t* module)
{
    LOGD("%s current db %s, loading %s", __func__, module->hal->current_db_file, TEST_TEMPLATE_DB);
    int status = fpc_tee_load_template_db(module->hal->bio, TEST_TEMPLATE_DB);
    if (status) {
        LOGE("%s fpc_tee_load_template_db failed %d", __func__, status);
    }
    return status;
}

static void restore_regular_db(engineering_module_t* module)
{
    LOGD("%s loading %s", __func__, module->hal->current_db_file);
    int status = fpc_tee_load_template_db(module->hal->bio, module->hal->current_db_file);
    if (status) {
        LOGE("%s fpc_tee_load_template_db failed %d", __func__, status);
    }
}

static int capture_images(fpc_engineering_t* self, fpc_capture_data_t* capture_data)
{
    LOGD("%s", __func__);
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    if (self->is_img_inj_enabled(self)) {
        status = self->handle_image_injection(self);
    } else {
        // Disable early stop
        uint8_t early_stop = CAC_EARLY_STOP_DISABLE;
        int es_status = fpc_tee_early_stop_ctrl(module->hal->sensor, &early_stop);

        if (es_status) {
            LOGE("%s setting early stop failed %d", __func__, es_status);
        }
        status = fpc_tee_capture_image(module->hal->sensor);
        capture_data->cac_result = fpc_tee_get_last_cac_result(module->hal->sensor);

        if (!es_status) {
            // Restore early stop setting
            es_status = fpc_tee_early_stop_ctrl(module->hal->sensor, &early_stop);
            if (es_status) {
                LOGE("%s restoring early stop failed %d", __func__, es_status);
            }
        }
    }

    switch (status) {
    case FPC_CAPTURE_OK:
        capture_data->capture_result = HAL_COMPAT_ACQUIRED_GOOD;

        status = fpc_hal_ext_get_raw_image(self, &capture_data->raw_image);

        if (status) {
            break;
        }

        status = fpc_hal_ext_get_enhanced_image(self, &capture_data->enhanced_image);

        break;
    case FPC_CAPTURE_FINGER_LOST:
        capture_data->capture_result = HAL_COMPAT_ACQUIRED_TOO_FAST;
        break;
    case FPC_CAPTURE_BAD_QUALITY:
        capture_data->capture_result = HAL_COMPAT_ACQUIRED_INSUFFICIENT;
        break;
    case -FPC_ERROR_IO:
        capture_data->capture_result = -FPC_ERROR_IO;
        break;
    case -FPC_ERROR_CANCELLED:
        capture_data->capture_result = -FPC_ERROR_CANCELLED;
        break;
    default:
        capture_data->capture_result = status;
        break;
    }

    return status;
}

static int clear_enroll_challenge(engineering_module_t* module)
{
    int status = 0;

#ifdef FPC_CONFIG_HW_AUTH
    uint64_t challenge = 0;
    status = fpc_tee_get_enrol_challenge(module->hal->tee_handle, &challenge);
    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        status = -EIO;
    }
#else
    (void) module;
#endif

    return status;
}

static int get_enroll_challenge(fpc_engineering_t* self, uint64_t* challenge)
{
    LOGD("%s", __func__);
    int status = 0;

#ifdef FPC_CONFIG_HW_AUTH
    engineering_module_t* module = (engineering_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    status = fpc_tee_get_enrol_challenge(module->hal->tee_handle, challenge);

    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        challenge = 0;
    }

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
#else
    (void) challenge;
    (void) self;
#endif

    return status;
}

static int set_enroll_token(fpc_engineering_t* self, const uint8_t* token, ssize_t token_length)
{
    LOGD("%s", __func__);

    int status = 0;

#ifdef FPC_CONFIG_HW_AUTH
    engineering_module_t* module = (engineering_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    status = fpc_tee_authorize_enrol(module->hal->tee_handle, token, token_length);

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
#else
    (void) self;
    (void) token_length;
    (void) token;
#endif

    if (status) {
        LOGE("%s failed %i\n", __func__, status);
    }

    return status;
}

static void execute_capture_callback(engineering_module_t* module,
                                     fpc_capture_data_t* capture_data)
{
    fpc_capture_cb_t capture_cb = module->capture_cb;
    void* capture_cb_ctx = module->capture_cb_ctx;
    if (!capture_data->samples_remaining) {
        module->capture_cb_ctx = NULL;
        module->capture_cb = NULL;
    }

    if(capture_cb && capture_cb_ctx) {
        capture_cb(capture_cb_ctx, capture_data);
    } else {
        LOGE("%s callback was NULL", __func__);
    }
}

static void do_capture(void* data)
{
    LOGD("%s", __func__);
    engineering_module_t* module = (engineering_module_t*) data;

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_CAPTURE;

    capture_images((fpc_engineering_t*)data, &capture_data);

    execute_capture_callback(module, &capture_data);
    free_captured_images(&capture_data);
}

static void do_verify(void* data)
{
    LOGD("%s", __func__);

    int status = 0;
    uint32_t id, update;

    engineering_module_t* module = (engineering_module_t*) data;
    status = load_test_db(module);
    if(status) {
        goto out;
    }

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_VERIFY;

    status = capture_images((fpc_engineering_t*)data, &capture_data);

    if(status) {
        goto out;
    }

    status = fpc_tee_identify(module->hal->bio, &id);
    capture_data.identify_result = status;

    if (status != FPC_CAPTURE_OK) {
        goto out;
    }

    capture_data.user_id = id;

    status = fpc_tee_update_template(module->hal->bio, &update);

    if (status) {
        goto out;
    }

    capture_data.template_update_result = status;

    if (update != 0) {
        save_test_db(module);
    }

out:
    execute_capture_callback(module, &capture_data);

    free_captured_images(&capture_data);
    restore_regular_db(module);
}

static void do_enroll(void* data)
{
    LOGD("%s", __func__);

    int status = 0;
    engineering_module_t* module = (engineering_module_t*) data;
    status = delete_test_db(module);
    if (status) {
        goto error;
    }
    load_test_db(module);

    fpc_capture_data_t capture_data;
    memset(&capture_data, 0, sizeof(capture_data));
    capture_data.mode = FPC_CAPTURE_MODE_ENROLL;

    status = fpc_tee_begin_enrol(module->hal->bio);

    if(status) {
        capture_data.enroll_result = status;
        LOGE("%s fpc_tee_begin_enrol failed %d", __func__, status);
        goto error;
    }

    while(true) {
        bool image_captured = false;
        while (!image_captured) {
            memset(&capture_data, 0, sizeof(capture_data));
            capture_data.mode = FPC_CAPTURE_MODE_ENROLL;

            status = capture_images((fpc_engineering_t*)data, &capture_data);
            LOGD("%s: capture_images, status=%d", __func__, status);
            switch (status) {
            case FPC_CAPTURE_QUALIFY_ABORT:
            case FPC_CAPTURE_FINGER_STUCK:
                LOGD("%s %s", __func__, status == FPC_CAPTURE_QUALIFY_ABORT ?
                        "FPC_CAPTURE_QUALIFY_ABORT" : "FPC_CAPTURE_FINGER_STUCK");
                capture_data.samples_remaining = REMAINING_UNKNOWN;
                execute_capture_callback(module, &capture_data);
                continue;
            case FPC_CAPTURE_OK:
                // Image captured OK and will be sent below.
                image_captured = true;
                break;
            case FPC_CAPTURE_BAD_QUALITY:
                LOGE("%s: FPC_CAPTURE_BAD_QUALITY", __func__);
                // We have an image, but should not try to enroll
                // we should also abort (as we do this is "regular" enroll)
                goto error;
            case FPC_CAPTURE_FINGER_LOST:
                capture_data.samples_remaining = REMAINING_UNKNOWN;
                execute_capture_callback(module, &capture_data);
                continue;
            default:
                goto error;
            }
        }

        status = fpc_tee_enrol(module->hal->bio, (uint32_t*)&capture_data.samples_remaining);
        capture_data.enroll_result = status;
        LOGD("%s samples_remaining=%d", __func__, capture_data.samples_remaining);

        if (status < 0 || status == FPC_ENROL_FAILED_COULD_NOT_COMPLETE
                || status == FPC_ENROL_FAILED_ALREADY_ENROLED) {
            LOGE("%s fpc_tee_enrol failed %d", __func__, status);
            goto error;
        }

        uint32_t id = 0;
        if (status == FPC_ENROL_COMPLETED) {
            status  = fpc_tee_end_enrol(module->hal->bio, &id);
            if (status) {
                LOGE("%s fpc_tee_end_enrol failed %d", __func__, status);
                capture_data.enroll_result = status;
                goto error;
            }

            status = save_test_db(module);

            if (status) {
                LOGE("%s save_test_db failed %d", __func__, status);
                capture_data.enroll_result = status;
                goto error;
            }
            capture_data.user_id = id;
        }
        execute_capture_callback(module, &capture_data);
        free_captured_images(&capture_data);

        if (capture_data.samples_remaining == 0) {
            goto out;
        }
    }
error:
    capture_data.samples_remaining = 0;
    execute_capture_callback(module, &capture_data);
out:
    free_captured_images(&capture_data);
    restore_regular_db(module);
    clear_enroll_challenge(module);
    LOGD("%s exit", __func__);
}

static int start_capture(fpc_engineering_t* self, fpc_capture_cb_t callback, fpc_capture_mode_t mode, void *ctx)
{
    LOGD("%s", __func__);
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);

    module->capture_cb = callback;
    module->capture_cb_ctx = ctx;

    LOGD("%s, using capture mode: %d", __func__, mode);

    if(mode == FPC_CAPTURE_MODE_CAPTURE) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_capture, (void*) module, FPC_TASK_HAL_EXT);
    } else if(mode == FPC_CAPTURE_MODE_VERIFY) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_verify, (void*) module, FPC_TASK_HAL_EXT);
    } else if(mode == FPC_CAPTURE_MODE_ENROLL) {
        fingerprint_hal_goto_idle(module->hal);
        fingerprint_hal_do_async_work(module->hal, do_enroll, (void*) module, FPC_TASK_HAL_EXT);
    }

    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static int cancel_capture(fpc_engineering_t* self)
{
    LOGD("%s", __func__);

    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static bool is_img_inj_enabled(fpc_engineering_t* self)
{
    engineering_module_t* module = (engineering_module_t*) self;
    bool enabled = false;
    pthread_mutex_lock(&module->mutex);

    if (module->img_inj_cb) {
        enabled = true;
    }

    pthread_mutex_unlock(&module->mutex);
    return enabled;
}

static int set_img_inj_cb(fpc_engineering_t* self,
                          fpc_img_inj_cb_t injection_callback,
                          fpc_img_inj_cancel_cb_t cancel_callback, void *ctx)
{
    LOGD("%s\n", __func__);
    int status = 0;
    engineering_module_t* module = (engineering_module_t*) self;
    pthread_mutex_lock(&module->mutex);

    if (injection_callback && cancel_callback) {
        module->img_inj_cb = injection_callback;
        module->img_inj_cancel_cb = cancel_callback;
        module->img_inj_cb_ctx = ctx;
    } else {
        module->img_inj_cb = NULL;
        module->img_inj_cancel_cb = NULL;
        module->img_inj_cb_ctx = NULL;
    }

    pthread_mutex_unlock(&module->mutex);
    return status;
}

static int handle_image_injection(fpc_engineering_t* self)
{
    LOGD("%s\n", __func__);
    int status = -1;

    fpc_hal_img_data_t image_data;
    memset(&image_data, 0, sizeof(fpc_hal_img_data_t));

    engineering_module_t* module = (engineering_module_t*) self;
    pthread_mutex_lock(&module->mutex);

    if (module->img_inj_cb) {
        module->is_inj_cancel = false;
        module->is_inj_wait_for_img = true;
        pthread_mutex_unlock(&module->mutex);

        // TODO: Fix module->img_inj_cb could be altered after mutex is unlocked
        status = module->img_inj_cb(module->img_inj_cb_ctx, &image_data);

        pthread_mutex_lock(&module->mutex);
        module->is_inj_wait_for_img = false;
        if (module->is_inj_cancel) {
            status = -FPC_ERROR_CANCELLED;
        }
    }
    pthread_mutex_unlock(&module->mutex);

    if (status == 0) {
        if (image_data.buffer != NULL) {
            status = fpc_tee_debug_inject(module->hal->tee_handle,
                    image_data.buffer, image_data.buffer_size);
        } else {
            status = -FPC_ERROR_IO;
            LOGE("%s failed to get fpc image data\n", __func__);
        }
    } else if (status != -FPC_ERROR_CANCELLED) {
        status = -FPC_ERROR_IO;
    }

    if (image_data.buffer != NULL) {
        free(image_data.buffer);
    }

    return status;
}

static void cancel_image_injection(fpc_engineering_t* self)
{
    engineering_module_t* module = (engineering_module_t*) self;
    pthread_mutex_lock(&module->mutex);

    if (module->img_inj_cancel_cb && module->is_inj_wait_for_img) {
        module->is_inj_cancel = true;
        module->img_inj_cancel_cb(module->img_inj_cb_ctx);
    }

    pthread_mutex_unlock(&module->mutex);
}

fpc_engineering_t* fpc_engineering_new(fpc_hal_common_t* hal)
{
    engineering_module_t* module = malloc(sizeof(engineering_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(engineering_module_t));
    module->hal = hal;

    pthread_mutex_lock(&module->hal->lock);

    int status = fpc_tee_get_sensor_info(module->hal->tee_handle,
                                         &module->sensor_width,
                                         &module->sensor_height);

    pthread_mutex_unlock(&module->hal->lock);

    if (status) {
        LOGE("%s: fpc_tac_get_sensor_info failed: %i\n", __func__, status);
        goto err;
    }

    module->engineering.get_sensor_size = get_sensor_size;
    module->engineering.set_img_subscr_cb = set_img_subscr_cb;
    module->engineering.handle_image_subscription = handle_image_subscription;
    module->engineering.handle_image_subscription_enroll = handle_image_subscription_enroll;
    module->engineering.handle_image_subscription_auth = handle_image_subscription_auth;
    module->engineering.is_img_inj_enabled = is_img_inj_enabled;
    module->engineering.set_img_inj_cb = set_img_inj_cb;
    module->engineering.handle_image_injection = handle_image_injection;
    module->engineering.cancel_image_injection = cancel_image_injection;
    module->engineering.start_capture = start_capture;
    module->engineering.cancel_capture = cancel_capture;
    module->engineering.set_enroll_token = set_enroll_token;
    module->engineering.get_enroll_challenge = get_enroll_challenge;
    pthread_mutex_init(&module->mutex, NULL);

    return (fpc_engineering_t*) module;

err:
    free(module);
    return NULL;
}

void fpc_engineering_destroy(fpc_engineering_t* self)
{
    if (!self) {
        return;
    }

    engineering_module_t* module = (engineering_module_t*) self;
    pthread_mutex_destroy(&module->mutex);
    free(self);
}
