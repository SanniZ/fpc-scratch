/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stddef.h>

#include "fpc_log.h"
#include "fpc_types.h"

#include "fpc_ta_interface.h"
#include "fpc_ta_engineering_interface.h"
#include "fpc_ta_module.h"
#include "fpc_ta_common.h"
#include "fpc_image_storage.h"
#include "fpc_ta_targets.h"

#ifndef NO_SENSOR
#include "fpc_ta_bio.h"
#include "fpc_ta_bio_internal.h"
#include "fpc_algo.h"
#endif


static int fpc_ta_engineering_retrieve(
    uint32_t type,
    void *buffer,
    size_t buffer_size)
{
    int status = 0;

    switch (type) {
        case FPC_TA_ENGINEERING_TYPE_RAW: {
            fpc_ta_common_t *common = fpc_common_get_handle();

            status = fpc_storage_retrieve_raw(
                common->image, (uint8_t*)buffer, buffer_size);
        } break;
        case FPC_TA_ENGINEERING_TYPE_ENHANCED_IMAGE: {
#ifdef NO_SENSOR
            /* Not supported */
            status = -1;
#else
            fpc_ta_common_t* common = fpc_common_get_handle();
            fpc_bio_t* bio = fpc_bio_get_handle();

            fpc_algo_storage_retrieve_enhanced_image(bio->algo_context,
            common->image, (uint8_t*)buffer, buffer_size);
#endif
            break;
        }
    }

    return status;
}

static int fpc_ta_engineering_get_sensor_info(
    uint8_t *width,
    uint8_t *height)
{
    if(NULL == width || NULL == height)
    {
        return -FPC_ERROR_INPUT;
    }

    fpc_common_get_sensor_info(width, height);
    return 0;
}

static int fpc_ta_engineering_inject_raw(
    const void *buffer,
    size_t buffer_size)
{

    fpc_ta_common_t* common = fpc_common_get_handle();

    if (fpc_storage_inject_raw((uint8_t*) buffer, buffer_size, common->image)) {
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

static int fpc_ta_engineering_get_raw_size(uint32_t *size)
{
    fpc_ta_common_t *common = fpc_common_get_handle();
    return fpc_storage_get_raw_size(common->image, size);
}

static int fpc_ta_engineering_handler(void *buffer, uint32_t size_buffer)
{
    fpc_ta_engineering_command_t *command =
        shared_cast_to(fpc_ta_engineering_command_t, buffer, size_buffer);

    if (!command) {
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s command %u", __func__, command->header.command);

    switch (command->header.command)
    {
    case FPC_TA_ENGINEERING_RETRIEVE:
        command->retrieve.response =
            fpc_ta_engineering_retrieve(
                command->retrieve.type,
                command->retrieve.array,
                command->retrieve.size);
        break;
    case FPC_TA_ENGINEERING_GET_SENSOR_INFO:
        command->sensor_info.response =
            fpc_ta_engineering_get_sensor_info(
                &command->sensor_info.width,
                &command->sensor_info.height);
        break;
    case FPC_TA_ENGINEERING_INJECT_RAW:
        command->inject.response =
            fpc_ta_engineering_inject_raw(
                command->inject.array,
                command->inject.size);
        break;
    case FPC_TA_ENGINEERING_GET_RAW_SIZE: {
        command->raw_size.response =
            fpc_ta_engineering_get_raw_size(
                &command->raw_size.size);
    } break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

static int fpc_ta_engineering_init(void)
{
    if(!fpc_common_is_engineering_supported())
    {
        LOGE("fpc_lib build does not support engineering.");
        return -FPC_ERROR_CONFIG;
    }

    return 0;
}

fpc_ta_module_t fpc_ta_engineering_module = {
    .init = fpc_ta_engineering_init,
    .exit = NULL,
    .handle_message = fpc_ta_engineering_handler,
    .key = TARGET_FPC_TA_ENGINEERING,
};
