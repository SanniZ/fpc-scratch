/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_PRIVATE_H
#define FPC_HAL_PRIVATE_H

#include <stdint.h>

#include "fpc_tee_hal.h"

#include "fpc_worker.h"
#include "fpc_tee.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_hw_auth.h"
#include "fpc_hal_ext_sensortest.h"
#include "fpc_hal_ext_engineering.h"
#include "fpc_hal_ext_authenticator.h"
#ifdef FPC_CONFIG_NAVIGATION
#include "fpc_hal_ext_navigation.h"
#endif
#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
#include "fpc_hal_ext_calibration.h"
#include "fpc_hal_ext_recalibration.h"
#endif


typedef enum {
    FPC_TASK_HAL = 0, /* Standard android tasks */
    FPC_TASK_HAL_EXT, /* FPC extension tasks */
} fpc_task_owner_t;

struct fpc_hal_common {

    struct fpc_worker* worker;
    struct {
        void (*func)(void*);
        void* arg;
        fpc_task_owner_t owner;
    } current_task;

    pthread_mutex_t lock;

    struct fpc_hal_ext_sensortest* ext_sensortest;
    struct fpc_engineering* ext_engineering;
    struct fpc_authenticator* ext_authenticator;
    struct fpc_navigation* ext_navigation;
    struct fpc_calibration* ext_calibration;
    struct fpc_recalibration* ext_recalibration;
    struct fpc_sense_touch* ext_sensetouch;

    struct fpc_tee* tee_handle;
    struct fpc_tee_sensor* sensor;
    struct fpc_tee_bio* bio;
    uint32_t current_gid;
    uint64_t challenge;
    uint64_t user_id;
    uint64_t authenticator_id;
    uint32_t remove_fid;
    uint8_t hat[69];
    char current_db_file[PATH_MAX];
    const fpc_hal_compat_callback_t* callback;
    void* callback_context;
};



void fingerprint_hal_resume(fpc_hal_common_t* dev);

void fingerprint_hal_do_async_work(fpc_hal_common_t* dev,
                                   void (*func)(void*), void* arg,
                                   fpc_task_owner_t owner);

void fingerprint_hal_goto_idle(fpc_hal_common_t* dev);



#endif // FPC_HAL_PRIVATE_H

