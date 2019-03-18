/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_RECALIBRATION_H
#define FPC_HAL_EXT_RECALIBRATION_H

#include <inttypes.h>
#include "fpc_tee_hal.h"

typedef void (*fpc_recalibration_status_cb_t)(void* ctx, int32_t code,
        int32_t image_decision, int32_t image_quality, int32_t pn_quality,
        int32_t progress);
typedef void (*fpc_recalibration_error_cb_t)(void* ctx, int32_t error);

typedef struct fpc_recalibration fpc_recalibration_t;

enum {
    //needs to be in sync with FingerprintRecalibration.java
    FPC_PN_RECALIBRATION_STATUS_WAITING_FOR_INPUT = 0,
    FPC_PN_RECALIBRATION_STATUS_UPDATE = 1,
    FPC_PN_RECALIBRATION_STATUS_DONE = 2,
};

enum {
    //needs to be in sync with FingerprintRecalibration.java
    FPC_PN_RECALIBRATION_ERROR_AUTHORIZATION = 0,
    FPC_PN_RECALIBRATION_ERROR_TIMEOUT = 1,
    FPC_PN_RECALIBRATION_ERROR_FAILED = 2,
    FPC_PN_RECALIBRATION_ERROR_CANCELED = 3,
    FPC_PN_RECALIBRATION_ERROR_MEMORY = 4,
    FPC_PN_RECALIBRATION_ERROR_INTERNAL = 5,
};

struct fpc_recalibration {
    void (*set_recalibration_cb)(fpc_recalibration_t* self,
            fpc_recalibration_status_cb_t status_callback,
            fpc_recalibration_error_cb_t error_callback, void *ctx);
    int (*recalibrate_pn)(fpc_recalibration_t* self, const uint8_t* token, ssize_t token_length);
    int (*pre_recalibrate_pn)(fpc_recalibration_t* self, uint64_t* challenge);
    int (*cancel_recalibration)(fpc_recalibration_t* self);
};

fpc_recalibration_t* fpc_recalibration_new(fpc_hal_common_t* hal);

void fpc_recalibration_destroy(fpc_recalibration_t* self);

#endif // FPC_HAL_EXT_RECALIBRATION_H
