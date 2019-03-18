/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_SENSORTEST_H
#define FPC_TEE_SENSORTEST_H

#include <stdint.h>
#include "fpc_tee_sensor.h"

typedef enum {
    FPC_TEE_SENSORTEST_SELF_TEST,
    FPC_TEE_SENSORTEST_CHECKERBOARD_TEST,
    FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST,
    FPC_TEE_SENSORTEST_RESET_PIXEL_TEST,
    FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST,
    FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST,
    FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST,
    FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST,
    FPC_TEE_SENSORTEST_PN_IMAGE_TEST,
} fpc_tee_sensortest_test_t;

int fpc_tee_sensortest_is_test_supported(fpc_tee_sensor_t *sensor,
                                         fpc_tee_sensortest_test_t test,
                                         int32_t *is_supported);

int fpc_tee_sensortest_run_test(fpc_tee_sensor_t *sensor,
                                fpc_tee_sensortest_test_t test,
                                uint32_t *result);

int fpc_tee_sensortest_run_module_quality_test(fpc_tee_sensor_t *sensor,
                                               uint32_t stabilization_ms,
                                               uint32_t snr_limit_preset,
                                               uint32_t snr_cropping_left,
                                               uint32_t snr_cropping_top,
                                               uint32_t snr_cropping_right,
                                               uint32_t snr_cropping_bottom,
                                               uint32_t* result,
                                               uint32_t* snr,
                                               uint32_t* snr_error);

int fpc_tee_sensortest_capture_uncalibrated(fpc_tee_sensor_t *sensor);
#endif //FPC_TEE_SENSORTEST_H
