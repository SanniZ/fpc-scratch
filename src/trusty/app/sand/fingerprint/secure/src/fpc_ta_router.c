/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stddef.h>

#include "fpc_ta_targets.h"
#include "fpc_log.h"
#include "fpc_ta_bio.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_ta_router.h"
#include "fpc_ta_module.h"

static int ta_initialized = 0;

extern fpc_ta_module_t fpc_ta_common_module;
#ifndef FPC_CONFIG_NO_SENSOR
extern fpc_ta_module_t fpc_ta_sensor_module;
#endif
#ifndef FPC_CONFIG_NO_ALGO
extern fpc_ta_module_t fpc_ta_bio_module;
#endif
extern fpc_ta_module_t fpc_ta_kpi_module;

#ifdef FPC_CONFIG_NAVIGATION
extern fpc_ta_module_t fpc_ta_navigation_module;
#endif
#ifdef FPC_CONFIG_HW_AUTH
extern fpc_ta_module_t fpc_ta_hw_auth_module;
#endif
#ifdef FPC_CONFIG_ENGINEERING
extern fpc_ta_module_t fpc_ta_engineering_module;
#endif
#ifdef FPC_CONFIG_TA_FS
extern fpc_ta_module_t fpc_ta_fs_module;
#endif
#ifdef FPC_CONFIG_TA_DB_BLOB
extern fpc_ta_module_t fpc_ta_db_blob_module;
#endif
#ifdef FPC_CONFIG_QC_AUTH
extern fpc_ta_module_t fpc_ta_qc_auth_module;
#endif
#ifdef FPC_CONFIG_SENSORTEST
extern fpc_ta_module_t fpc_ta_sensortest_module;
#endif
#ifdef FPC_CONFIG_APNS
extern fpc_ta_module_t fpc_ta_pn_module;
#endif

static fpc_ta_module_t* s_module_list[] = {
        &fpc_ta_common_module,
        #ifndef FPC_CONFIG_NO_SENSOR
        &fpc_ta_sensor_module,
        #endif
        #ifndef FPC_CONFIG_NO_ALGO
        &fpc_ta_bio_module,
        #endif
        &fpc_ta_kpi_module,
        #ifdef FPC_CONFIG_NAVIGATION
        &fpc_ta_navigation_module,
        #endif
        #ifdef FPC_CONFIG_HW_AUTH
        &fpc_ta_hw_auth_module,
        #endif
        #ifdef FPC_CONFIG_ENGINEERING
        &fpc_ta_engineering_module,
        #endif
        #ifdef FPC_CONFIG_TA_FS
        &fpc_ta_fs_module,
        #endif
        #ifdef FPC_CONFIG_TA_DB_BLOB
        &fpc_ta_db_blob_module,
        #endif
        #ifdef FPC_CONFIG_QC_AUTH
        &fpc_ta_qc_auth_module,
        #endif
        #ifdef FPC_CONFIG_SENSORTEST
        &fpc_ta_sensortest_module,
        #endif
        #ifdef FPC_CONFIG_APNS
        &fpc_ta_pn_module,
        #endif
    };

#define NUMBER_OF_MODULES (sizeof(s_module_list) / sizeof(s_module_list[0]))

int fpc_ta_router_init()
{
    if (ta_initialized)
        return 0;

    for (unsigned i = 0; i < NUMBER_OF_MODULES; ++i) {
        if (s_module_list[i]->init) {
            int status = s_module_list[i]->init();
            if (status) {
                LOGE("%s Module %d.init failed: %x", __func__, i, status);
                return status;
            }
        }
    }

    ta_initialized = 1;
    return 0;
}

void fpc_ta_router_exit()
{
    for (unsigned i = 0; i < NUMBER_OF_MODULES; ++i) {
        if (s_module_list[i]->exit) {
            s_module_list[i]->exit();
        }
    }
    ta_initialized = 0;
}

int fpc_ta_route_command(void* shared_buffer, uint32_t size_buffer)
{
    if (!ta_initialized) {
        LOGE("%s ta not initialized", __func__);
        return -FPC_ERROR_NOT_INITIALIZED;
    }

    fpc_ta_cmd_header_t* command = shared_cast_to(fpc_ta_cmd_header_t,
                                                  shared_buffer, size_buffer);

    if (!command) {
        LOGE("%s invalid input", __func__);
        return -FPC_ERROR_INPUT;
    }

    for (uint32_t i = 0; i < NUMBER_OF_MODULES; ++i) {
        if (s_module_list[i]->key == command->target) {
            return s_module_list[i]->handle_message(shared_buffer, size_buffer);
        }
    }

    LOGE("%s unsupported target %i\n", __func__, command->target);
    return -FPC_ERROR_CONFIG;
}
