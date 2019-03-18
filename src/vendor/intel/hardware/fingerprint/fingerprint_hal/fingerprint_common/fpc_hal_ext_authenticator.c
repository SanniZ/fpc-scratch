/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include "fpc_hal_ext_authenticator.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "container_of.h"
#include "fpc_types.h"
#include "fpc_tee_qc_auth.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_sensor.h"
#include "fpc_hal_private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FPC_DB_NO_MATCH -0x40 // Result from fpc_db.h, NO MATCH

typedef struct {
    fpc_authenticator_t authenticator;

    void* verify_user_cb_ctx;
    fpc_verify_user_cb_t verify_user_cb;
    fpc_verify_user_help_cb_t verify_user_help_cb;

    uint8_t nonce[QC_AUTH_NONCE];
    char dst_app_name[QC_SEC_APP_NAME_LEN];

    fpc_hal_common_t* hal;
} authenticator_module_t;

static void authenticator_reset(authenticator_module_t* module)
{
    memset(module->nonce, 0, sizeof(module->nonce));
    memset(module->dst_app_name, 0, sizeof(module->dst_app_name));

    module->verify_user_cb_ctx = NULL;
    module->verify_user_cb = NULL;
    module->verify_user_help_cb = NULL;
}

static void do_verify_user(void* data)
{
    LOGD("%s", __func__);
    int status;
    authenticator_module_t* module = (authenticator_module_t*) data;
    fpc_tee_qc_auth_data_t qc_auth_data;
    fpc_verify_user_data_t verify_user_data;
    memset(&verify_user_data, 0, sizeof(verify_user_data));
    memset(&qc_auth_data, 0, sizeof(qc_auth_data));

    status = fpc_tee_set_qc_auth_nonce(module->hal->tee_handle,
            module->nonce, sizeof(module->nonce));
    if (status) {
        LOGD("%s fpc_tee_set_qc_auth_nonce returned status code: %i", __func__, status);
        goto out;
    }

    bool capture_ok = false;
    while (!capture_ok) {
        status = fpc_tee_capture_image(module->hal->sensor);

        switch (status) {
            case FPC_CAPTURE_OK:
                capture_ok = true;
                break;
            case FPC_CAPTURE_FINGER_LOST:
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                            HAL_COMPAT_ACQUIRED_TOO_FAST);
                break;
            case FPC_CAPTURE_BAD_QUALITY:
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                            HAL_COMPAT_ACQUIRED_INSUFFICIENT);
                break;
            case FPC_CAPTURE_QUALIFY_ABORT:
                LOGD("%s FPC_CAPTURE_QUALIFY_ABORT", __func__);
                module->verify_user_help_cb(module->verify_user_cb_ctx,
                            HAL_COMPAT_ACQUIRED_INSUFFICIENT);
                goto out;
            case -FPC_ERROR_IO:
                goto out;
            case -FPC_ERROR_CANCELLED:
                goto out;
            default:
                goto out;
        }
    }

    uint32_t id, update = 0;

    status = fpc_tee_identify(module->hal->bio, &id);
    if (status) {
        goto out;
    }

    status = fpc_tee_update_template(module->hal->bio, &update);
    if (status) {
        goto out;
    }

    if (update != 0) {
        fpc_tee_store_template_db(module->hal->bio, module->hal->current_db_file);
    }

    if (id != 0) {
        status = fpc_tee_get_qc_auth_result(module->hal->tee_handle,
                module->nonce, sizeof(module->nonce),
                module->dst_app_name, sizeof(module->dst_app_name),
                &qc_auth_data);
        if (status) {
            LOGD("%s fpc_tee_get_qc_auth_result returned status code: %i", __func__, status);
            goto out;
        }

        verify_user_data.result = qc_auth_data.result;
        verify_user_data.user_id = qc_auth_data.user_id;
        verify_user_data.entity_id = qc_auth_data.user_entity_id;
        verify_user_data.size_result_blob = qc_auth_data.encapsulated_result_length;
        verify_user_data.result_blob = qc_auth_data.encapsulated_result;

    } else {
        // Asm map QC_AUTH_CODE_ERR to NO_MATCH
        verify_user_data.result = QC_AUTH_CODE_ERR;
    }

out:
    if (status == -EINTR) {
        verify_user_data.result = QC_AUTH_CODE_CANCEL;
    }
    else if (status != 0) {
        verify_user_data.result = QC_AUTH_CODE_ERR;
    }
    module->verify_user_cb(module->verify_user_cb_ctx, verify_user_data);
    authenticator_reset(module);
}

static int verify_user(fpc_authenticator_t* self,
            const uint8_t *nonce, uint32_t size_nonce,
            const char *dst_app_name, uint32_t size_dst_app_name,
            void* ctx, fpc_verify_user_cb_t verify_user_cb,
            fpc_verify_user_help_cb_t verify_user_help_cb)
{
    LOGD("%s", __func__);

    int status = 0;
    authenticator_module_t* module = (authenticator_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);

    // Check parameters

    if (nonce == NULL) {
        LOGE("%s failed nonce is NULL", __func__);
        status = -EINVAL;
        goto err;
    }

    if (size_nonce != QC_AUTH_NONCE) {
        LOGE("%s failed bad size_nonce parameter", __func__);
        status = -EINVAL;
        goto err;
    }

    if (dst_app_name == NULL) {
        LOGE("%s failed dst_app_name is NULL", __func__);
        status = -EINVAL;
        goto err;
    }

    if (size_dst_app_name == 0 || size_dst_app_name >= QC_SEC_APP_NAME_LEN) {
        LOGE("%s failed size_dst_app_name is %d", __func__, size_dst_app_name);
        status = -EINVAL;
        goto err;
    }

    if (ctx == NULL) {
        LOGE("%s failed ctx is NULL", __func__);
        status = -EINVAL;
        goto err;
    }

    if (verify_user_cb == NULL) {
        LOGE("%s failed verify_user_cb is NULL", __func__);
        status = -EINVAL;
        goto err;
    }

    if (verify_user_help_cb == NULL) {
        LOGE("%s failed verify_user_help_cb is NULL", __func__);
        status = -EINVAL;
        goto err;
    }

    fingerprint_hal_goto_idle(module->hal);
    memcpy(module->nonce, nonce, QC_AUTH_NONCE);

    // Copy dst_app_name and append null terminator
    memcpy(module->dst_app_name, dst_app_name, size_dst_app_name);
    module->dst_app_name[size_dst_app_name] = '\0';

    module->verify_user_cb_ctx = ctx;
    module->verify_user_cb = verify_user_cb;
    module->verify_user_help_cb = verify_user_help_cb;

    LOGD("%s setting nonce 0x%2x with size %zu and app: %s with size %i",
            __func__, module->nonce[0], sizeof(module->nonce),
            module->dst_app_name, size_dst_app_name);

    fingerprint_hal_do_async_work(module->hal, do_verify_user, module, FPC_TASK_HAL_EXT);

err:
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

static void cancel(fpc_authenticator_t* self)
{
    LOGD("%s", __func__);
    authenticator_module_t* module = (authenticator_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

static int is_user_valid(fpc_authenticator_t* self,
                            uint64_t user_id,
                            bool* is_user_valid)
{
    LOGD("%s", __func__);
    int status = 0;
    uint32_t result = 0;
    authenticator_module_t* module = (authenticator_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    status = fpc_tee_is_user_valid(module->hal->tee_handle, user_id, &result);
    if (status) {
        LOGE("%s fpc_tee_is_user_valid returned error: %i", __func__, status);
        goto unlock;
    }
    *is_user_valid = (result != 0);

unlock:
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);

    return status;
}

fpc_authenticator_t* fpc_authenticator_new(fpc_hal_common_t* hal)
{
    authenticator_module_t* module = malloc(sizeof(authenticator_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(authenticator_module_t));
    module->hal = hal;
    module->authenticator.verify_user = verify_user;
    module->authenticator.cancel = cancel;
    module->authenticator.is_user_valid = is_user_valid;

    return (fpc_authenticator_t*) module;
}

void fpc_authenticator_destroy(fpc_authenticator_t* self)
{
    if (!self) {
        return;
    }

    free(self);
}

