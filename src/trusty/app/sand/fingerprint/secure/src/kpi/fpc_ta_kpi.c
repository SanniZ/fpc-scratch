/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <string.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_module.h"
#include "fpc_ta_kpi_interface.h"
#include "fpc_types.h"
#include "fpc_kpi.h"
#include "fpc_build_info.h"
#include "fpc_log.h"

/*
//Below definitions are used to measure execution time of ta functions.
//Simply call FPC_TA_START_TIME_MEASUREMENT() and FPC_TA_STOP_TIME_MEASUREMENT()
//within a scope to use.
#ifdef FPC_CONFIG_DEBUG
//Representation of a measurement point.
//high = high bits of running time.
//low = low bits of running time.
typedef struct {
    uint32_t start_time_high;
    uint32_t start_time_low;
    uint32_t stop_time_high;
    uint32_t stop_time_low;
} fpc_ta_time_measurement_t;

static fpc_ta_time_measurement_t time_meas = { 0 };
extern void fpc_get_timestamp(uint32_t *high, uint32_t *low);

#define SEC_DIVIDER 1000000

#define FPC_TA_START_TIME_MEASUREMENT() fpc_get_timestamp(&time_meas.start_time_high, \
                                                          &time_meas.start_time_low)
#define FPC_TA_STOP_TIME_MEASUREMENT() \
do { \
    if(time_meas.start_time_low) { \
        fpc_get_timestamp(&time_meas.stop_time_high, \
                          &time_meas.stop_time_low); \
        LOGD("%s executed in %u sec, %u us !", __func__, \
            ((time_meas.stop_time_low - time_meas.start_time_low) / SEC_DIVIDER), \
            (time_meas.stop_time_low - time_meas.start_time_low) % SEC_DIVIDER); \
    } else { \
        LOGE("No start data in measurement, please run FPC_TA_START_TIME_MEASUREMENT " \
             "before invoking FPC_TA_STOP_TIME_MEASUREMENT"); \
    } \
} while(0)
#else
#define FPC_TA_START_TIME_MEASUREMENT() { }
#define FPC_TA_STOP_TIME_MEASUREMENT() { }
#endif
*/

static int fpc_ta_get_build_info(uint32_t* build_info_size, uint8_t* build_info)
{
    int result = -FPC_ERROR_INPUT;
    fpc_build_info_t fpc_build_info;
    uint32_t outsize = *build_info_size;

    if(fpc_get_build_info(&fpc_build_info))
    {
        return -FPC_ERROR_INPUT;
    }

    if((fpc_build_info.info_size + fpc_build_info.type_size) > outsize) {
        LOGE("%s: buffer too small, need %u got %u", __func__, (fpc_build_info.info_size + fpc_build_info.type_size), outsize);
        return -FPC_ERROR_NOSPACE;
    }

    memcpy(build_info, fpc_build_info.info, fpc_build_info.info_size);
    build_info[fpc_build_info.info_size - 1] = '\n';

    memcpy(build_info + fpc_build_info.info_size, fpc_build_info.type, fpc_build_info.type_size);
    build_info[fpc_build_info.info_size + fpc_build_info.type_size - 1] = '\0';

    *build_info_size = fpc_build_info.info_size + fpc_build_info.type_size;
    result = 0;
    return result;
}

static int fpc_kpi_command_handler(void* buffer, uint32_t size_buffer)
{

    LOG_ENTER();
    int ret = 0;

    fpc_ta_kpi_command_t* command = shared_cast_to(fpc_ta_kpi_command_t,
                                               buffer, size_buffer);
    if (!command)
    {
        LOGE("%s, no command?", __func__);
        return -FPC_ERROR_INPUT;
    }

    switch (command->header.command)
    {
    case FPC_TA_KPI_STOP_CMD:
        command->kpi_ctrl.response = fpc_kpi_stop(
            command->kpi_ctrl.array,
            &command->kpi_ctrl.size);
        break;
    case FPC_TA_KPI_START_CMD:
        command->kpi_ctrl.response = fpc_kpi_start();
        break;
    case FPC_TA_GET_BUILD_INFO_CMD:
        command->build_info.response = fpc_ta_get_build_info(
            &command->build_info.size, command->build_info.array);
        break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return ret;
}

fpc_ta_module_t fpc_ta_kpi_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_kpi_command_handler,
    .key = TARGET_FPC_TA_KPI,
};
