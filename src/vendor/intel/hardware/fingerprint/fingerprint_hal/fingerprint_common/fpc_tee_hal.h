/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_TEE_HAL_H
#define FPC_TEE_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <pthread.h>
#include <limits.h>

typedef enum {
    HAL_COMPAT_ERROR_NO_ERROR = 0,
    HAL_COMPAT_ERROR_HW_UNAVAILABLE = 1,
    HAL_COMPAT_ERROR_UNABLE_TO_PROCESS = 2,
    HAL_COMPAT_ERROR_TIMEOUT = 3,
    HAL_COMPAT_ERROR_NO_SPACE = 4,
    HAL_COMPAT_ERROR_CANCELED = 5,
    HAL_COMPAT_ERROR_UNABLE_TO_REMOVE = 6,
    HAL_COMPAT_ERROR_LOCKOUT = 7,
} fpc_hal_compat_error_t;

#define HAL_COMPAT_VENDOR_BASE 1000
#define HAL_COMPAT_ACQUIRED_TOO_SIMILAR (HAL_COMPAT_VENDOR_BASE + 0)
#define HAL_COMPAT_ERROR_ALREADY_ENROLLED (HAL_COMPAT_VENDOR_BASE + 0)

typedef enum {
    HAL_COMPAT_ACQUIRED_GOOD = 0,
    HAL_COMPAT_ACQUIRED_PARTIAL = 1,
    HAL_COMPAT_ACQUIRED_INSUFFICIENT = 2,
    HAL_COMPAT_ACQUIRED_IMAGER_DIRTY = 3,
    HAL_COMPAT_ACQUIRED_TOO_SLOW = 4,
    HAL_COMPAT_ACQUIRED_TOO_FAST = 5,
} fpc_hal_compat_acquired_t;

typedef struct fpc_hal_common fpc_hal_common_t;

typedef struct {
    void (*on_enroll_result)(void* context, uint32_t fid, uint32_t gid,
                             uint32_t remaining);
    void (*on_acquired)(void* context, int code);
    void (*on_authenticated)(void* context, uint32_t fid, uint32_t gid,
                             const uint8_t* token, uint32_t size_token);
    void (*on_error)(void* context, int code);
    void (*on_removed)(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);
    void (*on_enumerate)(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);
} fpc_hal_compat_callback_t;

int fpc_hal_open(fpc_hal_common_t** device,
             const fpc_hal_compat_callback_t* callback, void* callback_context);
void fpc_hal_close(fpc_hal_common_t* device);
uint64_t fpc_pre_enroll(fpc_hal_common_t* device);
int fpc_post_enroll(fpc_hal_common_t* device);
uint64_t fpc_get_authenticator_id(fpc_hal_common_t* device);
int fpc_set_active_group(fpc_hal_common_t* device, uint32_t gid,
                         const char* store_path);

int fpc_authenticate(fpc_hal_common_t* device,
                     uint64_t operation_id, uint32_t gid);

int fpc_enroll(fpc_hal_common_t* device, const uint8_t* hat, uint32_t size_hat,
               uint32_t gid, uint32_t timeout_sec);

int fpc_cancel(fpc_hal_common_t* device);
int fpc_remove(fpc_hal_common_t* device, uint32_t gid, uint32_t fid);
int fpc_enumerate(fpc_hal_common_t* device);

#ifdef __cplusplus
}
#endif

#endif // FPC_TEE_HAL_H
