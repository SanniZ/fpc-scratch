/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee_internal.h"
#include "fpc_tee_sensortest.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_sensortest_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"

#define SENSORTEST_STABILIZATION_MS 500
#define USECS_PER_MSEC              1000

#define _FPC_SELFTEST_IRQ_FAIL     6
#define _FPC_ERROR_SENSOR          7

int fpc_tee_sensortest_is_test_supported(fpc_tee_sensor_t *sensor,
                                         fpc_tee_sensortest_test_t test,
                                         int32_t *is_supported) {
    LOGD("%s", __func__);

    int status = 0;
    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    command->header.target = TARGET_FPC_TA_SENSORTEST;

    switch(test) {
        case FPC_TEE_SENSORTEST_SELF_TEST:
        case FPC_TEE_SENSORTEST_CHECKERBOARD_TEST:
            // always supported
            *is_supported = 1;
            return status;

        case FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_IMAGE_QUALITY_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_RESET_PIXEL_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_RESET_PIXEL_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_RUBBER_STAMP_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_AFD_RUBBER_STAMP_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_MODULE_QUALITY_TEST_SUPPORTED;
            break;

        case FPC_TEE_SENSORTEST_PN_IMAGE_TEST:
            command->header.command = FPC_TA_SENSORTEST_IS_PN_IMAGE_TEST_SUPPORTED;
            break;

        default:
            LOGD("%s unknown command: %d", __func__, test);
            *is_supported = 0;
            return status;
    }

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (!status) {
        *is_supported = command->is_test_supported.response;
    }

    return status;
}

static int fpc_tee_sensortest_test_command(fpc_tee_sensor_t* sensor, int32_t command_id, uint32_t *result)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    command->header.command   = command_id;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    command->test.result = *result; //send normal side irq-pin status to secure side
    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }
    *result = command->test.result;
    return command->test.response;
}

static int fpc_tee_sensortest_self_test(fpc_tee_sensor_t *sensor, uint32_t *result)
{
    int status;
    uint32_t temp_result;

    LOGD("%s", __func__);

    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_INIT, &temp_result);
    if (status) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_INIT failed! Status = %d", __func__, status);
        status = _FPC_ERROR_SENSOR;
        goto clean;
    }

    // Status is the value of irq pin, we expect 0 here after the init has been called
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ INITIAL -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ INITIAL -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_TA_SENSORTEST_TEST_OK;
        goto clean;
    }

    // Do selftest including irq tests on secure-side
    status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST, result);

    // If failure on TA side skip REE side checks they will give nothing as we don't know IRQ status
    if (*result || status) {
        LOGE("%s Selftest failed status = %d result = %d", __func__, status, *result);
        goto clean;
    }

    // After selftest is executed on TA we expect an active IRQ -> pin high
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ AFTER -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status == 0) {
        LOGE("%s IRQ TEST READ AFTER -> FPC_SELFTEST_IRQ_FAIL pin = %d should be > 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_TA_SENSORTEST_TEST_OK;
        goto clean;
    }

    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed", __func__);
        status = _FPC_ERROR_SENSOR;
        goto out;
    }

    // After cleanup selftest is executed on TA we expect no active IRQ -> pin low
    status = fpc_tee_status_irq(sensor);
    if (status < 0) {
        status = _FPC_ERROR_SENSOR;
        LOGE("%s IRQ TEST READ END -> FPC_ERROR_SENSOR", __func__);
        goto clean;
    } else if (status > 0) {
        LOGE("%s IRQ TEST READ END -> FPC_SELFTEST_IRQ_FAIL pin = %d should be 0",
             __func__, status);
        *result = _FPC_SELFTEST_IRQ_FAIL;
        status = FPC_TA_SENSORTEST_TEST_OK;
        goto clean;
    }

    goto out;

clean:
    if (fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_SELF_TEST_CLEANUP, &temp_result)) {
        LOGE("%s FPC_TA_SENSORTEST_SELF_TEST_CLEANUP failed, ignoring", __func__);
    }

out:
    return status;
}

int fpc_tee_sensortest_run_test(fpc_tee_sensor_t *sensor,
                                fpc_tee_sensortest_test_t test,
                                uint32_t *result) {
    LOGD("%s test: %d", __func__, test);
    int status = 0;

    switch(test) {
        case FPC_TEE_SENSORTEST_SELF_TEST:
            status = fpc_tee_sensortest_self_test(sensor, result);
            break;

        case FPC_TEE_SENSORTEST_CHECKERBOARD_TEST:
            status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_CHECKERBOARD_TEST, result);
            break;

        case FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST:
            status = fpc_tee_wait_finger_down(sensor);
            if (!status) {
                status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_IMAGE_QUALITY_TEST, result);
            }
            break;

        case FPC_TEE_SENSORTEST_RESET_PIXEL_TEST:
            status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_RESET_PIXEL_TEST, result);
            break;

        case FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST:
            status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_AFD_CALIBRATION_TEST, result);
            break;

        case FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST:
            status = fpc_tee_wait_finger_down(sensor);
            if (!status) {
                usleep(SENSORTEST_STABILIZATION_MS * USECS_PER_MSEC);
                status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST, result);
            }
            break;

        case FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST:
            status = fpc_tee_wait_finger_down(sensor);
            if (!status) {
                usleep(SENSORTEST_STABILIZATION_MS * USECS_PER_MSEC);
                status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_AFD_RUBBER_STAMP_TEST, result);
            }
            break;

        case FPC_TEE_SENSORTEST_PN_IMAGE_TEST:
            status = fpc_tee_sensortest_test_command(sensor, FPC_TA_SENSORTEST_PN_IMAGE_TEST, result);
            break;

        default:
            LOGD("%s unknown command: %d", __func__, test);
            status = FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
            break;
    }

    return status;
}

int fpc_tee_sensortest_run_module_quality_test(fpc_tee_sensor_t* sensor,
                                               uint32_t stabilization_ms,
                                               uint32_t snr_limit_preset,
                                               uint32_t snr_cropping_left,
                                               uint32_t snr_cropping_top,
                                               uint32_t snr_cropping_right,
                                               uint32_t snr_cropping_bottom,
                                               uint32_t* result,
                                               uint32_t* snr,
                                               uint32_t* snr_error)
{
    LOGD("%s", __func__);
    int status = fpc_tee_wait_finger_down(sensor);
    if (!status) {
        usleep(stabilization_ms * USECS_PER_MSEC);

        fpc_tee_t* tee = sensor->tee;
        fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
        command->header.command   = FPC_TA_SENSORTEST_MODULE_QUALITY_TEST;
        command->header.target    = TARGET_FPC_TA_SENSORTEST;
        command->module_quality_test.snr_limit_preset = snr_limit_preset;
        command->module_quality_test.snr_cropping_left = snr_cropping_left;
        command->module_quality_test.snr_cropping_top = snr_cropping_top;
        command->module_quality_test.snr_cropping_right = snr_cropping_right;
        command->module_quality_test.snr_cropping_bottom = snr_cropping_bottom;

        status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
        if (status) {
            return status;
        }

        *result = command->module_quality_test.result;
        *snr = command->module_quality_test.snr;
        *snr_error = command->module_quality_test.snr_error;
        return command->test.response;
    }

    return status;
}

int fpc_tee_sensortest_capture_uncalibrated(fpc_tee_sensor_t *sensor)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;
    fpc_ta_sensortest_command_t* command = (fpc_ta_sensortest_command_t*) tee->shared_buffer->addr;
    command->header.command   = FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED;
    command->header.target    = TARGET_FPC_TA_SENSORTEST;
    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    return command->capture_uncalibrated.response;
}
