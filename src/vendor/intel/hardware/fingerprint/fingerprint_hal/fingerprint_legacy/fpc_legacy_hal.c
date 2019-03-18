/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <errno.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#include "fpc_legacy_hal.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"

#ifdef FINGERPRINT_MODULE_API_VERSION_2_1
#define FPC_FINGERPRINT_MODULE_VERSION FINGERPRINT_MODULE_API_VERSION_2_1
#else
#define FPC_FINGERPRINT_MODULE_VERSION FINGERPRINT_MODULE_API_VERSION_2_0
#endif

static bool has_api_2_1(void)
{
#ifdef FINGERPRINT_MODULE_API_VERSION_2_1
    return true;
#else
    return false;
#endif
}

static uint64_t hal_pre_enroll(struct fingerprint_device *device)
{
   return fpc_pre_enroll(((fpc_legacy_hal_t*) device)->hal);
}

static int hal_post_enroll(struct fingerprint_device *device)
{
    return fpc_post_enroll(((fpc_legacy_hal_t*) device)->hal);
}

static uint64_t hal_get_authenticator_id(struct fingerprint_device *device)
{
    return fpc_get_authenticator_id(((fpc_legacy_hal_t*) device)->hal);
}

static int hal_set_active_group(struct fingerprint_device *device, uint32_t gid,
                                const char *store_path)
{
    return fpc_set_active_group(((fpc_legacy_hal_t*) device)->hal,
                                gid, store_path);
}

static int hal_authenticate(struct fingerprint_device *device,
                            uint64_t operation_id, uint32_t gid)
{
    return fpc_authenticate(((fpc_legacy_hal_t*) device)->hal,
                            operation_id, gid);
}

static int hal_enroll(struct fingerprint_device *device,
                      const hw_auth_token_t* hat,
                      uint32_t gid, uint32_t timeout_sec)
{
    return fpc_enroll(((fpc_legacy_hal_t*) device)->hal,
                      (const uint8_t*) hat, 69, gid, timeout_sec);
}

static int hal_cancel(struct fingerprint_device *device)
{
    return fpc_cancel(((fpc_legacy_hal_t*) device)->hal);
}

static int hal_remove(struct fingerprint_device *device, uint32_t gid, uint32_t fid)
{
    return fpc_remove(((fpc_legacy_hal_t*) device)->hal, gid, fid);
}

#ifdef FINGERPRINT_MODULE_API_VERSION_2_1
static int hal_enumerate(struct fingerprint_device* device)
{
    (void) device; // unused
    LOGD("%s", __func__);
    return 0;
}

#else
static int hal_enumerate(struct fingerprint_device *device,
                         fingerprint_finger_id_t *results,
                         uint32_t *max_size)
{
    (void)device; // Unused
    (void)results; // Unused
    (void)max_size; // Unused
    LOGD("%s", __func__);
    return 0;
}
#endif

static void on_enroll_result(void* context, uint32_t fid, uint32_t gid,
                         uint32_t remaining)
{
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) context;
    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_TEMPLATE_ENROLLING;
    msg.data.enroll.finger.fid = fid;
    msg.data.enroll.finger.gid = gid;
    msg.data.enroll.samples_remaining = remaining;
    dev->device.notify(&msg);
}

static void on_acquired(void* context, int code)
{
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) context;
    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_ACQUIRED;
    msg.data.acquired.acquired_info = (fingerprint_acquired_info_t) code;
    dev->device.notify(&msg);
}

static void on_authenticated(void* context, uint32_t fid, uint32_t gid,
                             const uint8_t* token, uint32_t size_token)
{
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) context;
    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_AUTHENTICATED;
    msg.data.authenticated.finger.fid = fid;
    msg.data.authenticated.finger.gid = gid;
    if (fid != 0) {
        if (size_token != sizeof(msg.data.authenticated.hat)) {
            LOGE("%s token size mismatch", __func__);
            return;
        }
        memcpy(&msg.data.authenticated.hat, token, size_token);
    }
    dev->device.notify(&msg);
}

static void on_error(void* context, int code)
{
#ifndef FINGERPRINT_MODULE_API_VERSION_2_1
    if (code == HAL_COMPAT_ERROR_CANCELED) {
       return;
    }
#endif
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) context;
    fingerprint_msg_t msg;
    msg.type = FINGERPRINT_ERROR;
    msg.data.error = (fingerprint_error_t) code;
    dev->device.notify(&msg);
}

static void on_removed(void* context, uint32_t fid, uint32_t gid,
                   uint32_t remaining)
{
    (void) remaining;
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) context;

    // Android M is only API2.0 and should not send a notification with fid=0
    if (!(!has_api_2_1() && fid == 0)) {
        fingerprint_msg_t msg;
        msg.type = FINGERPRINT_TEMPLATE_REMOVED;
        msg.data.removed.finger.fid = fid;
        msg.data.removed.finger.gid = gid;
#ifdef FINGERPRINT_MODULE_API_VERSION_3_0
        msg.data.removed.remaining_templates = remaining;
#endif
        dev->device.notify(&msg);
    }
}

static void on_enumerate(void* context, uint32_t fid, uint32_t gid,
                   uint32_t remaining)
{
    (void) context;
    (void) fid;
    (void) gid;
    (void) remaining;
}

static int set_notify(struct fingerprint_device *device,
        fingerprint_notify_t notify)
{
    LOGD("%s", __func__);

    device->notify = notify;
    return 0;
}

static int fpc_module_close(hw_device_t* device)
{
    LOGD("%s", __func__);

    if (!device) {
        return 0;
    }

    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) device;
    fpc_hal_close(dev->hal);
    free(device);

    return 0;
}

static int fpc_module_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    (void) name;
    fpc_legacy_hal_t* dev = (fpc_legacy_hal_t*) malloc(sizeof (*dev));

    if (!dev) {
        return -ENOMEM;
    }

    dev->callback.on_enroll_result = on_enroll_result;
    dev->callback.on_acquired = on_acquired;
    dev->callback.on_authenticated = on_authenticated;
    dev->callback.on_error = on_error;
    dev->callback.on_removed = on_removed;
    dev->callback.on_enumerate = on_enumerate;

    dev->hal = NULL;

    if (fpc_hal_open(&dev->hal, &dev->callback, dev)) {
        goto err;
    }

    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = FPC_FINGERPRINT_MODULE_VERSION;
    dev->device.common.module = (struct hw_module_t*) module;
    dev->device.common.close = fpc_module_close;
    dev->device.enroll = hal_enroll;
    dev->device.cancel = hal_cancel;
    dev->device.remove = hal_remove;
    dev->device.set_notify = set_notify;
    dev->device.notify = NULL;
    dev->device.authenticate = hal_authenticate;
    dev->device.pre_enroll = hal_pre_enroll;
    dev->device.enumerate = hal_enumerate;
    dev->device.get_authenticator_id = hal_get_authenticator_id;
    dev->device.set_active_group = hal_set_active_group;
    dev->device.post_enroll = hal_post_enroll;

    *device = (hw_device_t*) dev;

    return 0;
err:
    fpc_module_close((hw_device_t*)dev);

    return -1;
}

static struct hw_module_methods_t fpc_module_methods = {
    fpc_module_open
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    {
        HARDWARE_MODULE_TAG,                /* TAG */
        FPC_FINGERPRINT_MODULE_VERSION,     /* Module API version*/
        0,                                  /* HW API version */
        FINGERPRINT_HARDWARE_MODULE_ID,     /* ID */
        "FPC Fingrprint HAL",               /* Module name */
        "Fingerprint Cards AB",             /* Module author */
        &fpc_module_methods,                /* Module methods */
        0,                                  /* dso */
        {0,},                               /* reserved */
    },
};
