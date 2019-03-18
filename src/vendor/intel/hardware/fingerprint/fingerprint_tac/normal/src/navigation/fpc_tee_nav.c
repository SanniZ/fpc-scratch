/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_navigation_interface.h"
#include "fpc_types.h"
#include "fpc_tee_nav.h"
#include "fpc_ta_targets.h"
#include "fpc_nav_types.h"

int fpc_tee_nav_poll_data(fpc_tee_t* tee, fpc_nav_data_t* data)
{

    fpc_ta_nav_poll_data_cmd_t* command =
            (fpc_ta_nav_poll_data_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_POLL_DATA;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    data->finger_down = command->finger_down;
    data->nav_event = command->nav_event;
    data->request = command->request;
    data->force = command->force;

    return command->response;
}

int fpc_tee_nav_set_config(fpc_tee_t* tee, const fpc_nav_config_t* config)
{
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_SET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;
    command->config = *config;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    return command->response;
}

int fpc_tee_nav_get_config(fpc_tee_t* tee, fpc_nav_config_t* config)
{
    fpc_ta_nav_config_cmd_t* command =
            (fpc_ta_nav_config_cmd_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_GET_CONFIG;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    *config = command->config;

    return command->response;
}

int fpc_tee_nav_init(fpc_tee_t* tee)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_INIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    return command->init.response;
}

int fpc_tee_nav_exit(fpc_tee_t* tee)
{
    fpc_ta_navigation_command_t* command =
            (fpc_ta_navigation_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_NAVIGATION_EXIT;
    command->header.target = TARGET_FPC_TA_NAVIGATION;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    return command->init.response;
}
