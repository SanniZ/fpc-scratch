/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <string.h>
#include <fpc_mem.h>
#include "fpc_ta_sensor.h"
#include "fpc_ta_targets.h"
#include "fpc_ta_module.h"
#include "fpc_ta_sensor_interface.h"
#include "fpc_ta_common.h"

#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_crypto.h"
#include "fpc_test.h"
#include "fpc_sensor.h"
#include "fpc_otp.h"
#include "fpc_result.h"
#include "fpc_sensor_force.h"
#include "fpc_sensor_force_config.h"

static int fpc_ta_capture_image(fpc_device_capture_details_t *details);
static int fpc_ta_poll_qualify(void);

static device_context_t* _fpc_ta_sensor_context;

device_context_t* fpc_sensor_get_handle(void)
{
    return _fpc_ta_sensor_context;
}

static int fpc_ta_check_finger_lost(void)
{
    LOG_ENTER();
    fpc_ta_common_t *c_instance = fpc_common_get_handle();

    int status = fpc_sensor_check_finger_lost(_fpc_ta_sensor_context);

    switch (status) {
        case FPC_RESULT_FINGER_LOST:
            if (c_instance->check_for_bad_pixels) {
                if (SUCCESS(fpc_test_dead_pixel_update(c_instance->image))) {
                    c_instance->check_for_bad_pixels = 0;
                }
            }
            return 1;
        case FPC_RESULT_WAIT_TIME:
            return 0;
        case FPC_RESULT_ERROR_SENSOR:
            return -FPC_ERROR_RESET_HARDWARE;
        default:
            return -FPC_ERROR_IO;
    }
}

static int fpc_ta_finger_lost_wakeup_setup(void)
{
    int status;
    LOG_ENTER();

    status = fpc_sensor_finger_lost_wakeup_setup();

    switch (status) {

        case FPC_RESULT_OK:
            status = 0;
            break;

        case FPC_RESULT_ERROR_SENSOR:
            status = -FPC_ERROR_RESET_HARDWARE;
            break;

        default:
            status = -FPC_ERROR_IO;
    }

    return status;
}

static int fpc_ta_wakeup_setup(void)
{
    int status;

    LOG_ENTER();
    status = fpc_sensor_wakeup_setup();

    switch (status) {

        case FPC_RESULT_OK:
            status = 0;
            break;

        case FPC_RESULT_ERROR_SENSOR:
            status = -FPC_ERROR_RESET_HARDWARE;
            break;

        default:
            status = -FPC_ERROR_IO;
    }

    return status;
}

static int fpc_ta_qualify_capture(fpc_device_capture_details_t *details)
{
    LOG_ENTER();
    int status = fpc_sensor_wakeup_qualification();

    if ((status == FPC_RESULT_FINGER_LOST) ||
        (status == FPC_RESULT_WAIT_TIME)) {

        // Poll and qualify with timeout
        status = fpc_ta_poll_qualify();
    }

    switch (status) {

        case FPC_RESULT_FINGER_PRESENT:
            status = fpc_ta_capture_image(details);
            break;

        case FPC_RESULT_FINGER_LOST:
            status = FPC_CAPTURE_FINGER_LOST;
            break;
        case FPC_RESULT_WAIT_TIME:
            status = FPC_CAPTURE_QUALIFY_ABORT;
            break;

        case FPC_RESULT_ERROR_SENSOR:
            status = -FPC_ERROR_RESET_HARDWARE;
            break;

        default:
            status = -FPC_ERROR_IO;
    }

    return status;
}

/*
 * Poll sensor with Finger present query command with a timeout
 * of 30ms and check the number of zones active. The sensor is
 * required to be setup for finger present query command prior
 * to invoking this API.
 *
 *
* @return[out] FPC_RESULT_FINGER_PRESENT - if zone meet required number of zones
 *                                         configured for system.
 *             FPC_RESULT_FINGER_LOST    - if zero zones are detected.
 *             FPC_RESULT_WAIT_TIME      - if number of zones detected are
 *                                         less than required zones.
 *              < 0                      - Error
 *
 */
static int fpc_ta_poll_qualify(void)
{
    int ret = FPC_RESULT_WAIT_TIME;
    uint64_t time_start_us   = 0;
    uint64_t time_current_us = 0;
    uint32_t count           = 0;
    uint32_t expected_zones = 0;
    uint32_t current_zones  = 0;

    // 30 ms is the primary target to exit polling
    const uint64_t timeout_us = 30 * 1000;
    /*
     * Backup to the time polling.
     * 20 retries is ~30 ms measured in a bullhead. A
     * margin(* 3) is added on top of this for potentially faster
     * system
     */
    const uint32_t max_retries  = 60;

    ret = fpc_sensor_get_finger_detect_limit(&expected_zones);

    if (FAILED(ret)) {
        goto out;
    }

    fpc_get_timestamp(&time_start_us);
    time_current_us = time_start_us;

    while(((time_current_us - time_start_us) < timeout_us) && (count++ < max_retries)) {

        ret = fpc_sensor_get_dfd_zone_count(&current_zones);

        if (FAILED(ret)) {
            break;
        }

        if ((uint32_t) current_zones >= expected_zones) {
            LOGD("%s Finger Present!", __func__);
            ret = FPC_RESULT_FINGER_PRESENT;
            break;
        } else if (0 == current_zones) {
            ret = FPC_RESULT_FINGER_LOST;
        } else {
            ret = FPC_RESULT_WAIT_TIME;
        }

        fpc_get_timestamp(&time_current_us);
    }

    if (count >= max_retries) {
        // System is too fast than expected or there is an error in clock.
        // Continue with processing, but an indication to check.
        LOGD("%s Warning: Max retries reached", __func__);
    }

out:
    LOGD("<--%s %d", __func__, ret);

    return ret;
}

static int fpc_ta_capture_image(fpc_device_capture_details_t *details)
{
    LOG_ENTER();

    fpc_ta_common_t *c_instance = fpc_common_get_handle();

    c_instance->image_timestamp = fpc_get_uptime();
    int status = fpc_device_capture_image_status_details(
        _fpc_ta_sensor_context,
        c_instance->image,
        details);

    switch (status) {

        case FPC_RESULT_OK:
            status = FPC_CAPTURE_OK;
            break;

        case FPC_RESULT_FINGER_LOST:
            status = FPC_CAPTURE_FINGER_LOST;
            break;

        case FPC_RESULT_ERROR_SENSOR:
            status = -FPC_ERROR_RESET_HARDWARE;
            break;

        default:
            status =-FPC_ERROR_IO;
    }

    return status;
}

static int fpc_ta_get_force_value(uint8_t* value)
{
    LOG_ENTER();
    int status = 0;

    if (fpc_force_sensor_available()) {
        status = fpc_force_sensor_get_value(value);
    } else {
        status = FPC_ERROR_HARDWARE;
    }

    return status;
}

static int fpc_ta_deep_sleep(void)
{
    LOG_ENTER();
    int status = fpc_sensor_deep_sleep();

    switch (status) {

        case FPC_RESULT_OK:
            status = 0;
            break;

        case FPC_RESULT_ERROR_SENSOR:
            LOGE("%s deep sleep failed with error %i", __func__ , status);
            status = -FPC_ERROR_RESET_HARDWARE;
            break;

        default:
            LOGE("%s deep sleep failed with error %i", __func__ , status);
            status = -FPC_ERROR_IO;
            break;

    }

    return status;
}

static int fpc_ta_sensor_command_handler(void* buffer, uint32_t size_buffer)
{
    LOG_ENTER();
    fpc_ta_sensor_command_t* command = shared_cast_to(fpc_ta_sensor_command_t,
                                               buffer, size_buffer);
    if (!command) {
        LOGE("%s, no command?", __func__);
        return -FPC_ERROR_INPUT;
    }

    if (FAILED(fpc_sensor_communication_start())) {
        LOGE("<--%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }

    int ret = 0;

    switch (command->header.command) {

    case FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD:
        command->sensor.response = fpc_ta_check_finger_lost();
        break;
    case FPC_TA_SENSOR_FINGER_LOST_WAKEUP_SETUP_CMD:
        command->sensor.response = fpc_ta_finger_lost_wakeup_setup();
        break;
    case FPC_TA_SENSOR_WAKEUP_SETUP_CMD:
        command->sensor.response = fpc_ta_wakeup_setup();
        break;
    case FPC_TA_SENSOR_CAPTURE_IMAGE_CMD:
    {
        fpc_device_capture_details_t details;
        memset(&details, 0, sizeof(details));
        command->capture_info.header.response = fpc_ta_qualify_capture(&details);
        command->capture_info.cac_result = details.cac_result;
        break;
    }
    case FPC_TA_SENSOR_DEEP_SLEEP_CMD:
        command->sensor.response = fpc_ta_deep_sleep();
        break;
    case FPC_TA_SENSOR_IS_OTP_SUPPORTED_CMD:
        command->is_otp_supported.response = fpc_otp_is_supported();
        break;
    case FPC_TA_SENSOR_GET_OTP_INFO_CMD:
        command->otp_info.response = fpc_otp_get_sensor_info(&command->otp_info.data);
        break;
#ifdef FPC_CONFIG_ENGINEERING
    case FPC_TA_SENSOR_EARLY_STOP_CTRL_CMD:
        command->early_stop_ctrl.header.response =
                fpc_device_early_stop_ctrl(_fpc_ta_sensor_context, &command->early_stop_ctrl.ctrl);
        break;
#endif
    case FPC_TA_SENSOR_GET_FORCE_VALUE:
        command->get_force_value.header.response =
                fpc_ta_get_force_value(&command->get_force_value.value);
        break;
    case FPC_TA_SENSOR_IS_SENSOR_FORCE_SUPPORTED:
        command->is_sensor_force_supported.response = fpc_force_sensor_available();
        break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        ret = -FPC_ERROR_INPUT;
    }

    if (FAILED(fpc_sensor_communication_stop())) {
        ret = -FPC_ERROR_RESET_HARDWARE;
    }

    return ret;
}

static int fpc_ta_sensor_init(void)
{
    LOG_ENTER();
    int status = 0;
    uint32_t dev_context_size = 0;
    fpc_ta_common_t *c_instance = fpc_common_get_handle();

    if (FAILED(fpc_sensor_communication_start())) {
        LOGE("<--%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }

    status = fpc_device_init(NULL, &dev_context_size);
    if(FPC_RESULT_ERROR_MEMORY == status)
    {
        _fpc_ta_sensor_context = (device_context_t*)malloc(dev_context_size);
        if (!_fpc_ta_sensor_context) {
             LOGE("<--%s fail to malloc dev_context_size\n", __func__);
             return -FPC_ERROR_ALLOC;
         }
        status = fpc_device_init(_fpc_ta_sensor_context, &dev_context_size);
    }

    if (FAILED(status)) {
        LOGE("<--%s device init failed %d.", __func__, status);
        status = -FPC_ERROR_RESET_HARDWARE;
        goto out;
    }

    // do deadpixel update when getting fingerlost.
    c_instance->check_for_bad_pixels = 1;

out:
    if (FAILED(fpc_sensor_communication_stop())) {
        status = -FPC_ERROR_RESET_HARDWARE;
    }

    return status;
}

static void fpc_ta_sensor_exit(void)
{
    LOG_ENTER();
    fpc_device_deinit(_fpc_ta_sensor_context);
    free(_fpc_ta_sensor_context);

}

fpc_ta_module_t fpc_ta_sensor_module = {
    .init = fpc_ta_sensor_init,
    .exit = fpc_ta_sensor_exit,
    .handle_message = fpc_ta_sensor_command_handler,
    .key = TARGET_FPC_TA_SENSOR,
};
