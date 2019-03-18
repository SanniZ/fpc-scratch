/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stddef.h>

#include "fpc_log.h"
#include "fpc_types.h"

#include "fpc_ta_bio.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_sensortest_interface.h"
#include "fpc_ta_module.h"
#include "fpc_ta_targets.h"
#include "fpc_ta_common.h"
#include "fpc_ta_sensor.h"

#include "fpc_test.h"
#include "fpc_sensor.h"
#include "fpc_result.h"

static int map_result(int code)
{
    switch (code) {
    case FPC_RESULT_OK:
        return FPC_TA_SENSORTEST_TEST_OK;
    case FPC_RESULT_ERROR_NOT_SUPPORTED:
        return FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
    case FPC_RESULT_ERROR_MEMORY:
        return -FPC_ERROR_ALLOC;
    default:
        return -FPC_ERROR_HARDWARE;
    }
}

static int fpc_ta_sensortest_handler(void *buffer, uint32_t size_buffer)
{
    int ret = 0;
    LOGD("%s", __func__);

    fpc_ta_sensortest_command_t *command =
        shared_cast_to(fpc_ta_sensortest_command_t, buffer, size_buffer);

    if (!command) {
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s command %u", __func__, command->header.command);

    if (fpc_sensor_communication_start()) {
        LOGE("%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }

    int32_t response = 0;
    fpc_ta_common_t *common = fpc_common_get_handle();
    image_t *image = common->image;

    switch (command->header.command) {
    case FPC_TA_SENSORTEST_SELF_TEST_INIT:
        response = fpc_test_self_test_init(&command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_SELF_TEST:
        response = fpc_test_run_self_test(&command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_SELF_TEST_CLEANUP:
        response = fpc_test_self_test_cleanup(&command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_CHECKERBOARD_TEST:
        response = fpc_test_run_checkerboard_test(image, &command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_IMAGE_QUALITY_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_image_quality_test_supported();
        break;
    case FPC_TA_SENSORTEST_IMAGE_QUALITY_TEST:
        response = fpc_test_run_image_quality_test(image, &command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_RESET_PIXEL_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_reset_pixel_test_supported();
        break;
    case FPC_TA_SENSORTEST_RESET_PIXEL_TEST:
        response = fpc_test_run_reset_pixel_test(&command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_afd_test_supported(AFD_CALIBRATION_TEST);
        break;
    case FPC_TA_SENSORTEST_AFD_CALIBRATION_TEST:
        response = fpc_test_run_afd_test(AFD_CALIBRATION_TEST, &command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_RUBBER_STAMP_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_afd_test_supported(AFD_CALIBRATION_RUBBER_STAMP_TEST);
        break;
    case FPC_TA_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST:
        response = fpc_test_run_afd_test(AFD_CALIBRATION_RUBBER_STAMP_TEST, &command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_AFD_RUBBER_STAMP_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_afd_test_supported(AFD_RUBBER_STAMP_TEST);
        break;
    case FPC_TA_SENSORTEST_AFD_RUBBER_STAMP_TEST:
        response = fpc_test_run_afd_test(AFD_RUBBER_STAMP_TEST, &command->test.result);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_MODULE_QUALITY_TEST_SUPPORTED:
        command->is_test_supported.response = fpc_test_is_module_quality_test_supported();
        break;
    case FPC_TA_SENSORTEST_MODULE_QUALITY_TEST:
        response = fpc_test_run_module_quality_test(image,
                                                    command->module_quality_test.snr_limit_preset,
                                                    command->module_quality_test.snr_cropping_left,
                                                    command->module_quality_test.snr_cropping_top,
                                                    command->module_quality_test.snr_cropping_right,
                                                    command->module_quality_test.snr_cropping_bottom,
                                                    &command->module_quality_test.result,
                                                    &command->module_quality_test.snr,
                                                    &command->module_quality_test.snr_error);
        command->test.response = map_result(response);
        break;
    case FPC_TA_SENSORTEST_IS_PN_IMAGE_TEST_SUPPORTED: {
        command->is_test_supported.response = fpc_test_is_pn_image_test_supported();
        break;
    }
    case FPC_TA_SENSORTEST_PN_IMAGE_TEST: {
        response = fpc_test_run_pn_image_test(image, &command->test.result);
        command->test.response = map_result(response);
        break;
    }
    case FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED: {
        device_context_t *d_instance = fpc_sensor_get_handle();
        response = fpc_device_capture_uncalibrated_image(d_instance, image);
        command->capture_uncalibrated.response = map_result(response);
        }
        break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        ret = -FPC_ERROR_INPUT;
    }

    if (fpc_sensor_communication_stop()) {
        return -FPC_ERROR_RESET_HARDWARE;
    }

    return ret;
}

fpc_ta_module_t fpc_ta_sensortest_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_ta_sensortest_handler,
    .key = TARGET_FPC_TA_SENSORTEST,
};
