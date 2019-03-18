/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef FPC_TA_SENSORTEST_INTERFACE_H
#define FPC_TA_SENSORTEST_INTERFACE_H

#include "fpc_ta_interface.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint32_t result;
} fpc_ta_sensortest_test_cmd_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    uint32_t snr_limit_preset;
    uint32_t snr_cropping_left;
    uint32_t snr_cropping_top;
    uint32_t snr_cropping_right;
    uint32_t snr_cropping_bottom;
    uint32_t result;
    uint32_t snr;
    uint32_t snr_error;
} fpc_ta_sensortest_module_quality_test_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_sensortest_test_cmd_t test;
    fpc_ta_sensortest_module_quality_test_cmd_t module_quality_test;
    fpc_ta_simple_command_t is_test_supported;
    fpc_ta_simple_command_t capture_uncalibrated;
} fpc_ta_sensortest_command_t;

enum {
    FPC_TA_SENSORTEST_TEST_OK = 0,
    FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED = 1,
};

typedef enum {
    FPC_TA_SENSORTEST_SELF_TEST_INIT,
    FPC_TA_SENSORTEST_SELF_TEST,
    FPC_TA_SENSORTEST_SELF_TEST_CLEANUP,
    FPC_TA_SENSORTEST_CHECKERBOARD_TEST,
    FPC_TA_SENSORTEST_IS_IMAGE_QUALITY_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_IMAGE_QUALITY_TEST,
    FPC_TA_SENSORTEST_IS_RESET_PIXEL_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_RESET_PIXEL_TEST,
    FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_AFD_CALIBRATION_TEST,
    FPC_TA_SENSORTEST_IS_AFD_CALIBRATION_RUBBER_STAMP_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST,
    FPC_TA_SENSORTEST_IS_AFD_RUBBER_STAMP_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_AFD_RUBBER_STAMP_TEST,
    FPC_TA_SENSORTEST_IS_MODULE_QUALITY_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_MODULE_QUALITY_TEST,
    FPC_TA_SENSORTEST_IS_PN_IMAGE_TEST_SUPPORTED,
    FPC_TA_SENSORTEST_PN_IMAGE_TEST,
    FPC_TA_SENSORTEST_CAPTURE_UNCALIBRATED,
} fpc_ta_sensortest_cmd_t;

#endif // FPC_TA_SENSORTEST_INTERFACE_H

