/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#include <stddef.h>

#include "fpc_log.h"
#include "fpc_types.h"

#include "fpc_ta_sensor.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_navigation_interface.h"
#include "fpc_ta_module.h"
#include "fpc_ta_targets.h"
#include "fpc_ta_common.h"
#include "fpc_nav.h"

static int fpc_ta_navigation_handler(void *buffer, uint32_t size_buffer)
{
    fpc_ta_navigation_command_t *command =
        shared_cast_to(fpc_ta_navigation_command_t, buffer, size_buffer);

    if (!command) {
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s command %u", __func__, command->header.command);
    if (fpc_sensor_communication_start()) {
        LOGE("<--%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }

    int ret = 0;

    switch (command->header.command) {
        case FPC_TA_NAVIGATION_INIT: {
            fpc_nav_init_data_t init_data = {.context = fpc_sensor_get_handle(),
                                             .image = fpc_common_get_handle()->image};

            command->init.response = fpc_navigation_init(&init_data);
            break;
        }
        case FPC_TA_NAVIGATION_EXIT:
            command->exit.response = fpc_navigation_exit();
            break;
        case FPC_TA_NAVIGATION_POLL_DATA: {
            fpc_nav_data_t nav_data;
            command->poll_data.response = fpc_navigation_poll_data(&nav_data);
            command->poll_data.nav_event = nav_data.nav_event;
            command->poll_data.finger_down = nav_data.finger_down;
            command->poll_data.request = nav_data.request;
            command->poll_data.force = nav_data.force;
            break;
        }
        case FPC_TA_NAVIGATION_SET_CONFIG:
            command->set_config.response = fpc_navigation_set_config(&command->set_config.config);
            break;
        case FPC_TA_NAVIGATION_GET_CONFIG:
            command->get_config.response = fpc_navigation_get_config(&command->get_config.config);
            break;
        default:
            LOGE("%s unknown command %i", __func__, command->header.command);
            ret = -FPC_ERROR_INPUT;
    }

    if (fpc_sensor_communication_stop()) {
        LOGE("<--%s communication stop failed.", __func__);
        ret = -FPC_ERROR_RESET_HARDWARE;
    }

    return ret;
}

fpc_ta_module_t fpc_ta_navigation_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_ta_navigation_handler,
    .key = TARGET_FPC_TA_NAVIGATION,
};

