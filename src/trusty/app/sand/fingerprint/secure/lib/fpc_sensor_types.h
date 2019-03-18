/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_SENSOR_TYPES_H_
#define FPC_SENSOR_TYPES_H_
#include <stdint.h>

typedef enum {
    SENSOR_SELFTEST_SUCCESS,
    SENSOR_SELFTEST_FAIL,
    SENSOR_SELFTEST_POWER_WAKEUP_FAIL,
    SENSOR_SELFTEST_SENSOR_RESET_FAIL,
    SENSOR_SELFTEST_READ_HWID_FAIL,
    SENSOR_SELFTEST_CAPTURE_IMAGE_FAIL,
    SENSOR_SELFTEST_IRQ_FAIL,
    SENSOR_SELFTEST_SENSOR_COULD_NOT_BE_REACHED = 16,
} sensor_selftest_result_t;

typedef enum {
    SENSOR_STATUS_OK,
    SENSOR_WORKING,
    SENSOR_INITIALISING,
    SENSOR_OUT_OF_ORDER,
    SENSOR_MALFUNCTIONED,
    SENSOR_FAILURE,
} sensor_status_t;

#endif /* FPC_SENSOR_H_ */
