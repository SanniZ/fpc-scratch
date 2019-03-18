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
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <fcntl.h>
#include "container_of.h"

#include "fpc_tee.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_log.h"
#include "fpc_tee_hal.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_tee_engineering.h"
#include "fpc_tee_kpi.h"
#include "fpc_worker.h"
#include "fpc_hal_private.h"
#include "fpc_hal_input_device.h"

#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
#include "fpc_hal_ext_calibration_service.h"
#endif
#ifdef FPC_CONFIG_APNS
#include "fpc_hal_pn.h"
#include "fpc_hal_ext_recalibration_service.h"
#endif
#ifdef FPC_CONFIG_QC_AUTH
#include "fpc_hal_ext_authenticator_service.h"
#endif
#ifdef FPC_CONFIG_ENGINEERING
#include "fpc_hal_ext_engineering_service.h"
#include "fpc_ta_bio_interface.h"
#endif
#ifdef FPC_CONFIG_NAVIGATION
#include "fpc_hal_ext_navigation_service.h"
#include "fpc_hal_navigation.h"
#endif
#ifdef FPC_CONFIG_SENSORTEST
#include "fpc_hal_ext_sensortest_service.h"
#endif
#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
#include "fpc_hal_ext_sense_touch_service.h"
#include "fpc_hal_sense_touch.h"
#endif

#define TEMPLATE_POSTFIX                "/user.db"

// This should be same as MAX_FAILED_ATTEMPTS in FingerprintService.java
#define FPC_MAX_FAILED_ATTEMPTS 5

static unsigned int android_platform_version_major(void)
{
    return PLATFORM_VERSION_MAJOR;
}

static void hal_work_func(void* arg)
{
    fpc_hal_common_t* dev = (fpc_hal_common_t*) arg;
    dev->current_task.func(dev->current_task.arg);
#ifdef FPC_CONFIG_NAVIGATION
    fpc_navigation_resume(dev->ext_navigation);
#endif
}

void fingerprint_hal_do_async_work(fpc_hal_common_t* dev,
                                   void (*func)(void*), void* arg,
                                   fpc_task_owner_t owner)
{
    fpc_worker_join_task(dev->worker);
#ifdef FPC_CONFIG_NAVIGATION
    fpc_navigation_pause(dev->ext_navigation);
#endif
    dev->current_task.func = func;
    dev->current_task.arg = arg;
    dev->current_task.owner = owner;
    fpc_worker_run_task(dev->worker, hal_work_func, dev);
}

void fingerprint_hal_goto_idle(fpc_hal_common_t* dev)
{
    fpc_tee_set_cancel(dev->sensor);
#ifdef FPC_CONFIG_ENGINEERING
    if (dev->ext_engineering) {
        dev->ext_engineering->cancel_image_injection(dev->ext_engineering);
    }
#endif
    fpc_worker_join_task(dev->worker);
    fpc_tee_clear_cancel(dev->sensor);
#ifdef FPC_CONFIG_NAVIGATION
    fpc_navigation_pause(dev->ext_navigation);
#endif
}

void fingerprint_hal_resume(fpc_hal_common_t* dev)
{
    fpc_worker_join_task(dev->worker);
#ifdef FPC_CONFIG_NAVIGATION
    fpc_navigation_resume(dev->ext_navigation);
#endif
}

static int32_t capture_image(fpc_hal_common_t* dev)
{
    int32_t status = 0;

#ifdef FPC_CONFIG_ENGINEERING
    if (dev->ext_engineering->is_img_inj_enabled(dev->ext_engineering)) {
        status = dev->ext_engineering->handle_image_injection(dev->ext_engineering);
    } else {
#endif
        for(;;) {
            status = fpc_tee_capture_image(dev->sensor);

            switch(status) {
                case FPC_CAPTURE_OK: //0
                case FPC_CAPTURE_BAD_QUALITY:
                    goto out;
                case FPC_CAPTURE_FINGER_LOST:
                    dev->callback->on_acquired(dev->callback_context,
                                               HAL_COMPAT_ACQUIRED_TOO_FAST);
                    break;
                case -FPC_ERROR_CANCELLED:
                    goto out;
                case -FPC_ERROR_IO:
                    goto out;
                default:
                    goto out;
            }
        };
#ifdef FPC_CONFIG_ENGINEERING
    }
#endif
out:
    return status;
}

#if FPC_CONFIG_FORCE_SENSOR == 1
static int sense_touch_capture_image(fpc_hal_common_t* dev)
{
    const fpc_sense_touch_config_t* st_config = fpc_sense_touch_get_config();
    int status = capture_image(dev);

    if (status == FPC_CAPTURE_OK && st_config != NULL && st_config->auth_enable_down_force) {
        int result = fpc_tee_wait_for_button_down_force(dev->sensor,
                                                        st_config->auth_button_timeout_ms,
                                                        st_config->trigger_threshold);
        if (result == FPC_CAPTURE_OK) {
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_DOWN);
        } else if (result == -FPC_ERROR_TIMEDOUT || result == FPC_CAPTURE_FINGER_LOST) {
            status = FPC_CAPTURE_RETRY;
        } else {
            status = result;
        }
    }
    if (status == FPC_CAPTURE_OK && st_config != NULL && st_config->auth_enable_up_force) {

        int result = fpc_tee_wait_for_button_up_force(dev->sensor,
                                                      st_config->auth_button_timeout_ms,
                                                      st_config->untrigger_threshold);
        if (result == FPC_CAPTURE_OK) {
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_UP);
        } else if (result == -FPC_ERROR_TIMEDOUT) {
            report_input_event(FPC_SENSE_TOUCH_EVENT, FPC_SENSE_TOUCH_AUTH_PRESS, FPC_HAL_INPUT_KEY_UP);
            status = FPC_CAPTURE_RETRY;
        } else {
            status = result;
        }
    }
    return status;
}
#endif

static void do_authenticate(void* data)
{
    LOGD("%s", __func__);
    int status;
    /* Image qualifier check is enabled during authentication so we can make an early
    exit and disregard the image if sensor gets activated while in pocket. */
    uint32_t id = 0;
    uint32_t update = 0;
    uint32_t failed_attempts = 0;
    fpc_hal_common_t* dev = (fpc_hal_common_t*) data;

#ifdef FPC_CONFIG_HW_AUTH
    status = fpc_tee_set_auth_challenge(dev->tee_handle, dev->challenge);
    if (status) {
        goto out;
    }
#endif

    for (;;) {
        fpc_tee_kpi_start(dev->tee_handle);

#if FPC_CONFIG_FORCE_SENSOR == 1
        status = sense_touch_capture_image(dev);
        if (status == FPC_CAPTURE_RETRY) {
            continue;
        }
#else
        status = capture_image(dev);
#endif
        if ((status == FPC_CAPTURE_QUALIFY_ABORT) ||
            (status == FPC_CAPTURE_FINGER_STUCK)) {
            LOGD("%s %s", __func__, status == FPC_CAPTURE_QUALIFY_ABORT ?
                    "FPC_CAPTURE_QUALIFY_ABORT" : "FPC_CAPTURE_FINGER_STUCK");

            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            continue;
        } else if (status != FPC_CAPTURE_OK) {
#ifdef FPC_CONFIG_ENGINEERING
            if (status == FPC_CAPTURE_BAD_QUALITY) {
                dev->ext_engineering->handle_image_subscription_auth(dev->ext_engineering,
                        status, 0, COVERAGE_UNKNOWN, QUALITY_UNKNOWN, id);
            }
#endif
            goto out;
        }

        status = fpc_tee_identify(dev->bio, &id);
        if (status < 0) {
            /* A real error */
            goto out;
        }

#ifdef FPC_CONFIG_ENGINEERING
        fpc_ta_bio_identify_statistics_t stat;
        memset(&stat, 0, sizeof(fpc_ta_bio_identify_statistics_t));
        int status_get_statistics = fpc_tee_get_identify_statistics(dev->bio, &stat);
        if(status_get_statistics) {
            LOGD("%s failed to get identify statistics\n", __func__);
        }
        dev->ext_engineering->handle_image_subscription_auth(dev->ext_engineering, 0, status,
                stat.coverage, stat.quality, id);
#endif

        if (status == FPC_CAPTURE_BAD_QUALITY) {
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            continue;
        }

        dev->callback->on_acquired(dev->callback_context,
                                   HAL_COMPAT_ACQUIRED_GOOD);

        if (id != 0) {
            uint8_t hat[69];
#ifdef FPC_CONFIG_HW_AUTH
            status = fpc_tee_get_auth_result(dev->tee_handle, hat, sizeof(hat));

            if (status) {
                goto out;
            }
#endif
            dev->callback->on_authenticated(dev->callback_context,
                                            id, dev->current_gid,
                                            hat, sizeof(hat));

            status = fpc_tee_update_template(dev->bio, &update);
            if (status) {
                goto out;
            }

            if (update != 0) {
                fpc_tee_store_template_db(dev->bio, dev->current_db_file);
            }
            break;
        } else {
            dev->callback->on_authenticated(dev->callback_context,
                                            0, dev->current_gid, NULL, 0);

            status = fpc_tee_update_template(dev->bio, &update);
            if (status) {
                goto out;
            }

            if (update != 0) {
                fpc_tee_store_template_db(dev->bio, dev->current_db_file);
            }

            failed_attempts++;
            if (failed_attempts >= FPC_MAX_FAILED_ATTEMPTS) {
                LOGD("%s failed %d times\n", __func__, failed_attempts);
                status = -FPC_ERROR_CANCELLED;
                goto out;
            }
        }

        /* Recycle KPI to flush the kpi buffers */
        fpc_tee_kpi_stop(dev->tee_handle);
    }


out:
    /* Will not harm to stop again even if it's already stopped */
    fpc_tee_kpi_stop(dev->tee_handle);

    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        switch (status) {
        case -FPC_ERROR_CANCELLED:
            break;
        default:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_HW_UNAVAILABLE);
            break;
        }
    }
}

static void do_enroll(void* data)
{
    int status = 0;
    fpc_hal_common_t* dev = (fpc_hal_common_t*) data;

    uint32_t remaining_samples = 0;

#ifdef FPC_CONFIG_HW_AUTH
    status = fpc_tee_authorize_enrol(dev->tee_handle,
                                     dev->hat, sizeof(dev->hat));
    if (status) {
        goto out;
    }
#endif

    fpc_tee_kpi_start(dev->tee_handle);

    status = fpc_tee_begin_enrol(dev->bio);

    if (status) {
        goto out;
    }

    for(;;) {
        status = capture_image(dev);
        if (status == FPC_CAPTURE_QUALIFY_ABORT) {
            LOGD("%s FPC_CAPTURE_QUALIFY_ABORT", __func__);
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            continue;
        } else if (status == FPC_CAPTURE_FINGER_STUCK) {
            // Just keep on trying, user will sooner or later get tired of
            // holding the finger on the sensor.
            LOGD("%s FPC_CAPTURE_FINGER_STUCK", __func__);
            continue;
        } else if (status) {
#ifdef FPC_CONFIG_ENGINEERING
            if (status == FPC_CAPTURE_BAD_QUALITY) {
                dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering,
                        status, 0, REMAINING_UNKNOWN, 0);
            }
#endif
            goto out;
        }

        status = fpc_tee_enrol(dev->bio, &remaining_samples);
        if (status < 0) {
            goto out;
        }

#ifdef FPC_CONFIG_ENGINEERING
        if (status != FPC_ENROL_COMPLETED) {
            dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering, 0,
                    status, remaining_samples, 0);
        }
#endif

        uint32_t id = 0;
        switch (status) {
        case FPC_ENROL_COMPLETED:
            status  = fpc_tee_end_enrol(dev->bio, &id);
            if (status) {
                goto out;
            }

            status = fpc_tee_store_template_db(dev->bio,
                                               dev->current_db_file);

            if (status) {
                fpc_tee_load_template_db(dev->bio, dev->current_db_file);
                goto out;
            }

            if (fpc_tee_sensor_cancelled(dev->sensor)) {
                LOGD("%s canceled enroll\n", __func__);
                fpc_tee_delete_template(dev->bio, id);
                fpc_tee_store_template_db(dev->bio,
                                          dev->current_db_file);
                status = -FPC_ERROR_CANCELLED;
                goto out;
            }

            dev->callback->on_enroll_result(dev->callback_context,
                                            id, dev->current_gid, 0);

            status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);

            if (status) {
                LOGE("%s failed to get auth id %i\n", __func__, status);
                dev->authenticator_id = 0;
            }
            status = 0;
#ifdef FPC_CONFIG_ENGINEERING
            dev->ext_engineering->handle_image_subscription_enroll(dev->ext_engineering,
                    0, FPC_ENROL_COMPLETED, 0, id);
#endif
            goto out;
        case FPC_ENROL_PROGRESS:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_GOOD);
            dev->callback->on_enroll_result(dev->callback_context,
                                            0, dev->current_gid,
                                            remaining_samples);
            break;
        case FPC_ENROL_FAILED_COULD_NOT_COMPLETE:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
            status = 0;
            goto out;
        case FPC_ENROL_IMAGE_TOO_SIMILAR:
            dev->callback->on_enroll_result(dev->callback_context,
                                            0, dev->current_gid,
                                            remaining_samples);
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_TOO_SIMILAR);
            break;
        case FPC_ENROL_FAILED_ALREADY_ENROLED:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_ALREADY_ENROLLED);
            status = 0;
            goto out;
        case FPC_ENROL_IMAGE_LOW_QUALITY:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_IMAGER_DIRTY);
            break;
        case FPC_ENROL_IMAGE_LOW_COVERAGE:
            dev->callback->on_acquired(dev->callback_context,
                                       HAL_COMPAT_ACQUIRED_PARTIAL);
            break;
        }
    }

out:

    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        switch (status) {
        case -FPC_ERROR_CANCELLED:
            break;
        case -FPC_ERROR_TIMEDOUT:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_TIMEOUT);
            break;
        case -FPC_ERROR_NOSPACE:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_NO_SPACE);
            break;
        default:
            dev->callback->on_error(dev->callback_context,
                                    HAL_COMPAT_ERROR_HW_UNAVAILABLE);
            break;
        }
    }

    fpc_tee_kpi_stop(dev->tee_handle);

}

uint64_t fpc_pre_enroll(fpc_hal_common_t* dev)
{
    LOGD("%s\n", __func__);
    pthread_mutex_lock(&dev->lock);
    fingerprint_hal_goto_idle(dev);
    uint64_t challenge = 0;
#ifdef FPC_CONFIG_HW_AUTH
    int status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);

    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        challenge = 0;
    }

#endif
    LOGD("%s challenge %" PRIu64 "\n", __func__, challenge);

    fingerprint_hal_resume(dev);
    pthread_mutex_unlock(&dev->lock);

    return challenge;
}

int fpc_post_enroll(fpc_hal_common_t* dev)
{
    LOGD("%s\n", __func__);
    pthread_mutex_lock(&dev->lock);
    fingerprint_hal_goto_idle(dev);
    int status = 0;
#ifdef FPC_CONFIG_HW_AUTH
    uint64_t challenge = 0;
    status = fpc_tee_get_enrol_challenge(dev->tee_handle, &challenge);
    if (status) {
        LOGE("%s failed %i\n", __func__, status);
        status = -EIO;
    }

#endif
    fingerprint_hal_resume(dev);
    pthread_mutex_unlock(&dev->lock);

    return status;
}

uint64_t fpc_get_authenticator_id(fpc_hal_common_t* dev)
{
    LOGD("%s\n", __func__);

    pthread_mutex_lock(&dev->lock);
    uint64_t id = dev->authenticator_id;
    pthread_mutex_unlock(&dev->lock);

    return id;
}

int fpc_set_active_group(fpc_hal_common_t* dev, uint32_t gid,
                                const char *store_path)
{
    int status = 0;

    LOGD("%s\n", __func__);

    pthread_mutex_lock(&dev->lock);
    fingerprint_hal_goto_idle(dev);
    int length = snprintf(dev->current_db_file,
                          sizeof(dev->current_db_file), "%s%s",
                          store_path, TEMPLATE_POSTFIX);

    if (length < 0 || (unsigned) length >= sizeof(dev->current_db_file)) {
        status = -EINVAL;
        goto out;
    }

    status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
    if (status != 0) {
        LOGE("%s: fpc_tac_load_user_db failed with error %s", __func__, fpc_error_str(status));
        status = -1;
        goto out;
    }

    status = fpc_tee_set_gid(dev->bio, gid);
    if (status) {
        goto out;
    }

    dev->current_gid = gid;

    status = fpc_tee_get_template_db_id(dev->bio, &dev->authenticator_id);

    if (status) {
        LOGE("%s failed to get auth id %i\n", __func__, status);
        dev->authenticator_id = 0;
    }

out:
    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        status = -1;
    }

    fingerprint_hal_resume(dev);
    pthread_mutex_unlock(&dev->lock);

    return status;
}

int fpc_authenticate(fpc_hal_common_t* dev,
                            uint64_t operation_id, uint32_t gid)
{
    LOGD("%s operation_id %" PRIu64 "\n", __func__, operation_id);

    int status = 0;
    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s finger.gid != current_gid\n", __func__);
        status = -1;
        goto out;
    }

    fingerprint_hal_goto_idle(dev);
    dev->challenge = operation_id;
    fingerprint_hal_do_async_work(dev, do_authenticate, dev, FPC_TASK_HAL);
out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}

int fpc_enroll(fpc_hal_common_t* dev, const uint8_t* hat, uint32_t size_hat,
                      uint32_t gid, uint32_t timeout_sec)
{
    (void)timeout_sec; // Unused
    LOGD("%s", __func__);

    int status = 0;
    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s finger.gid != current_gid\n", __func__);
        status = -1;
        goto out;
    }

    if (size_hat != sizeof(dev->hat)) {
        LOGD("%s hat size mismatch %d", __func__, size_hat);
        status = -1;
        goto out;
    }

    memcpy(dev->hat, hat, size_hat);

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_enroll, dev, FPC_TASK_HAL);

out:
    pthread_mutex_unlock(&dev->lock);
    return status;
}

int fpc_cancel(fpc_hal_common_t* dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);
    if(dev->current_task.owner == FPC_TASK_HAL) {
        fingerprint_hal_goto_idle(dev);
        fingerprint_hal_resume(dev);
    }

    dev->callback->on_error(dev->callback_context,
                            HAL_COMPAT_ERROR_CANCELED);

    pthread_mutex_unlock(&dev->lock);
    return 0;
}

static void do_remove(void* device)
{
    fpc_hal_common_t* dev = (fpc_hal_common_t*) device;

    uint32_t ids[5];
    uint32_t size = 5;

    int status = 0;

    if (dev->remove_fid == 0) {
        status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
        if (status) {
            status = -EIO;
            goto out;
        }
    } else {
        ids[0] = dev->remove_fid;
        size = 1;
    }

    for (unsigned i = 0; i < size; ++i) {

        status = fpc_tee_delete_template(dev->bio, ids[i]);
        if (status != 0 && status != -FPC_ERROR_NOENTITY) {
            LOGE("%s delete_tempalte failed %i", __func__, status);
            status = -EIO;
            goto out;
        }

        if (status != -FPC_ERROR_NOENTITY) {
            status = fpc_tee_store_template_db(dev->bio,
                                               dev->current_db_file);
            if (status) {
                LOGE("%s store_template_db failed %i", __func__, status);
                status = -EIO;
                goto out;
            }
        }

        status = 0;
        dev->callback->on_removed(dev->callback_context,
                                  ids[i], dev->current_gid,
                                  size - 1 - i);
    }

    // We do not want to change behaviour in android 7 or earlier.
    if (android_platform_version_major() < 8 || dev->remove_fid == 0) {
        dev->callback->on_removed(dev->callback_context, 0, dev->current_gid, 0);
    }

out:

    if (status) {
        dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_REMOVE);

        LOGE("%s failed %i, reloading db\n", __func__, status);
        status = fpc_tee_load_template_db(dev->bio, dev->current_db_file);
        if (status != 0) {
            LOGE("%s: fpc_tac_load_user_db failed with error %s", __func__,
                 fpc_error_str(status));
        }
    }

}

int fpc_remove(fpc_hal_common_t* dev, uint32_t gid, uint32_t fid)
{
    int status = 0;

    LOGD("%s(fid=%d gid=%d)", __func__, fid, gid);

    pthread_mutex_lock(&dev->lock);

    if (gid != dev->current_gid) {
        LOGD("%s gid != current_gid, nothing to remove\n", __func__);
        status = -2;
        goto out;
    }

    dev->remove_fid = fid;

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_remove, dev, FPC_TASK_HAL);


out:
    pthread_mutex_unlock(&dev->lock);

    return status;
}

static void do_enumerate(void* device)
{
    fpc_hal_common_t* dev = (fpc_hal_common_t*) device;

    const int max_templates = 5;
    uint32_t ids[max_templates];
    uint32_t size = max_templates;

    int status = fpc_tee_get_finger_ids(dev->bio, &size, ids);
    if (status) {
        dev->callback->on_error(dev->callback_context,
                                HAL_COMPAT_ERROR_UNABLE_TO_PROCESS);
        LOGE("%s failed %s", __func__, fpc_error_str(status));
        return;
    }

    if (size == 0) {
        dev->callback->on_enumerate(dev->callback_context, 0,
                                    dev->current_gid, 0);
        return;
    }

    for (unsigned i = 0; i < size; ++i) {
        dev->callback->on_enumerate(dev->callback_context, ids[i],
                                    dev->current_gid, (size -1) - i);
    }

}

int fpc_enumerate(fpc_hal_common_t* dev)
{
    LOGD("%s", __func__);

    pthread_mutex_lock(&dev->lock);

    fingerprint_hal_goto_idle(dev);
    fingerprint_hal_do_async_work(dev, do_enumerate, dev, FPC_TASK_HAL);

    pthread_mutex_unlock(&dev->lock);

    return 0;
}

void fpc_hal_close(fpc_hal_common_t* device)
{
    LOGD("%s", __func__);

    if (!device) {
        return;
    }

    fpc_hal_common_t* dev = (fpc_hal_common_t*) device;

    pthread_mutex_lock(&dev->lock);

    if (dev->tee_handle) {
        if (dev->sensor) {
            fpc_tee_set_cancel(dev->sensor);
        }
    #ifdef FPC_CONFIG_ENGINEERING
        if (dev->ext_engineering) {
            dev->ext_engineering->cancel_image_injection(dev->ext_engineering);
        }
    #endif
        if (dev->worker) {
            fpc_worker_join_task(dev->worker);
        }

        if (dev->sensor) {
            fpc_tee_clear_cancel(dev->sensor);
        }
    #ifdef FPC_CONFIG_NAVIGATION
        if (dev->ext_navigation) {
            fpc_navigation_pause(dev->ext_navigation);
        }
    #endif
    }

    destroy_input_device();
    fpc_worker_destroy(dev->worker);

#ifdef FPC_CONFIG_SENSORTEST
    fpc_sensortest_destroy(dev->ext_sensortest);
#endif

#ifdef FPC_CONFIG_ENGINEERING
    fpc_engineering_destroy(dev->ext_engineering);
#endif

#ifdef FPC_CONFIG_QC_AUTH
    fpc_authenticator_destroy(dev->ext_authenticator);
#endif

#ifdef FPC_CONFIG_NAVIGATION
    fpc_navigation_destroy(dev->ext_navigation);
#endif

#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
    fpc_calibration_destroy(dev->ext_calibration);
#endif

#ifdef FPC_CONFIG_APNS
    fpc_recalibration_destroy(dev->ext_recalibration);
#endif

#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
    fpc_sense_touch_destroy(dev->ext_sensetouch);
#endif

    fpc_tee_sensor_release(dev->sensor);

    fpc_tee_bio_release(dev->bio);

    fpc_tee_release(dev->tee_handle);

    pthread_mutex_unlock(&dev->lock);
    pthread_mutex_destroy(&dev->lock);
    free(device);
}

int fpc_hal_open(fpc_hal_common_t** device,
                 const fpc_hal_compat_callback_t* callback,
                 void* callback_context)
{
    LOGD("%s", __func__);

    *device = NULL;

    fpc_hal_common_t *dev = (fpc_hal_common_t*)
        malloc(sizeof(fpc_hal_common_t));

    if (!dev) {
        return -ENOMEM;
    }

    memset(dev, 0, sizeof(fpc_hal_common_t));

    dev->callback = callback;
    dev->callback_context = callback_context;
    pthread_mutex_init(&dev->lock, NULL);

    dev->tee_handle = fpc_tee_init();
    if (!dev->tee_handle) {
        goto err;
    }

    if (create_input_device()) {
        LOGE("%s Failed to create input device.", __func__);
        goto err;
    }

    if(fpc_tee_print_build_info(dev->tee_handle)) {
        LOGD("%s, An error occured print build info.\n", __func__);
    }

    dev->sensor = fpc_tee_sensor_init(dev->tee_handle);
    if (!dev->sensor) {
        goto err;
    }

    dev->bio = fpc_tee_bio_init(dev->tee_handle);
    if (!dev->bio) {
        goto err;
    }

#ifdef FPC_CONFIG_HW_AUTH
    if (fpc_tee_init_hw_auth(dev->tee_handle)) {
        goto err;
    }
#endif

    dev->worker = fpc_worker_new();
    if (!dev->worker) {
        goto err;
    }

#ifdef FPC_CONFIG_ENGINEERING
    dev->ext_engineering = fpc_engineering_new(dev);
    if (!dev->ext_engineering) {
        goto err;
    }
    add_engineering_service(dev->ext_engineering);
#endif

#ifdef FPC_CONFIG_SENSORTEST
    dev->ext_sensortest = fpc_sensortest_new(dev, dev->ext_engineering);
    if (!dev->ext_sensortest) {
        goto err;
    }
    add_sensortest_service(dev->ext_sensortest);
#endif

#ifdef FPC_CONFIG_QC_AUTH
    dev->ext_authenticator = fpc_authenticator_new(dev);
    if (!dev->ext_authenticator) {
        goto err;
    }
    add_authenticator_service(dev->ext_authenticator);
#endif

#if defined(FPC_CONFIG_FORCE_SENSOR) || defined(FPC_CONFIG_NAVIGATION_FORCE_SW)
    dev->ext_sensetouch = fpc_sense_touch_new(dev);
    fpc_sense_touch_load_config();
    if(!dev->ext_sensetouch) {
        goto err;
    }
    add_sense_touch_service(dev->ext_sensetouch);
#endif

#ifdef FPC_CONFIG_NAVIGATION
    dev->ext_navigation = fpc_navigation_new(dev->tee_handle);
    if (!dev->ext_navigation) {
        goto err;
    }
    add_navigation_service(dev->ext_navigation);
#endif

#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
    dev->ext_calibration = fpc_calibration_new(dev);
    if (!dev->ext_calibration) {
        goto err;
    }
    add_calibration_service(dev->ext_calibration);
#endif

#ifdef FPC_CONFIG_APNS
    dev->ext_recalibration = fpc_recalibration_new(dev);
    if (!dev->ext_recalibration) {
        goto err;
    }
    add_recalibration_service(dev->ext_recalibration);

    fpc_load_pn(dev);
#endif

    *device = dev;

    fingerprint_hal_resume(dev);

    return 0;
err:
    LOGE("%s failed\n", __func__);
    fpc_hal_close(dev);
    return -1;
}
