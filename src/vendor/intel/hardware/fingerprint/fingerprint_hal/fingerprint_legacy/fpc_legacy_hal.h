#ifndef FPC_LEGACY_HAL_H
#define FPC_LEGACY_HAL_H

#include <pthread.h>
#include <limits.h>

#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#include "fpc_tee_hal.h"

#define FINGERPRINT_ACQUIRED_TOO_SIMILAR \
        ((fingerprint_acquired_info_t)FINGERPRINT_ACQUIRED_VENDOR_BASE + 0)

#define FINGERPRINT_ERROR_ALREADY_ENROLLED \
        ((fingerprint_acquired_info_t)FINGERPRINT_ERROR_VENDOR_BASE + 0)

typedef struct {
    fingerprint_device_t device; //inheritance

    fpc_hal_common_t* hal;
    fpc_hal_compat_callback_t callback;
} fpc_legacy_hal_t;


#endif // FPC_LEGACY_HAL_H

