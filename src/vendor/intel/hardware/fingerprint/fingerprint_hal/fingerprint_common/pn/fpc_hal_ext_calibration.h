/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_CALIBRATION_H
#define FPC_HAL_EXT_CALIBRATION_H

#include <inttypes.h>
#include "fpc_tee_hal.h"

typedef int (*fpc_calibration_status_cb_t)(void* ctx, uint8_t code);
typedef int (*fpc_calibration_error_cb_t)(void* ctx, int8_t error);

typedef struct fpc_calibration fpc_calibration_t;

enum {
    //needs to be in sync with FingerprintCalibration.java
    FPC_PN_CALIBRATION_STATUS_WAITING_FOR_INPUT = 0,
    FPC_PN_CALIBRATION_STATUS_STABILIZE = 1,
    FPC_PN_CALIBRATION_STATUS_START = 2,
    FPC_PN_CALIBRATION_STATUS_RETRY = 3,
    FPC_PN_CALIBRATION_STATUS_DONE = 4,
};

struct fpc_calibration {
    void (*set_calibration_cb)(fpc_calibration_t* self,
            fpc_calibration_status_cb_t progress_callback,
            fpc_calibration_error_cb_t error_callback, void *ctx);
    int (*calibrate_pn)(fpc_calibration_t* self);
};

fpc_calibration_t* fpc_calibration_new(fpc_hal_common_t* hal);

void fpc_calibration_destroy(fpc_calibration_t* self);

#endif // FPC_HAL_EXT_CALIBRATION_H
