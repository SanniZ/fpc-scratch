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
#include "fpc_ta_common.h"
#include "fpc_ta_common_interface.h"
#include "fpc_log.h"
#include "fpc_external.h"
#ifdef FPC_CONFIG_LOGGING_IN_RELEASE_FILE
#include "fpc_debug.h"
#endif

static fpc_ta_common_t g_fpc_common_data_instance;

static void* _fpc_common_buffer;

fpc_ta_common_t* fpc_common_get_handle()
{
    return &g_fpc_common_data_instance;
}


static int fpc_ta_common_command_handler(void* buffer, uint32_t size_buffer)
{

    fpc_ta_common_command_t *command = shared_cast_to(fpc_ta_common_command_t,
                                                buffer, size_buffer);
    if (!command) {
        LOGE("%s, Invalid command", __func__);
        return -FPC_ERROR_INPUT;
    }

    switch (command->header.command) {
        case FPC_TA_COMMON_GET_ERROR_LOG_CMD:

#ifdef FPC_CONFIG_LOGGING_IN_RELEASE_FILE
            fpc_debug_get_circular_log(command->error_msg.array,
                    &command->error_msg.size);
#endif
            break;
        default:
            LOGE("%s, this should never happen!", __func__);
            break;
    }

    return 0;
}

static int fpc_ta_common_init(void)
{
    LOG_ENTER();
    uint32_t size = fpc_common_get_info_size();

    _fpc_common_buffer = malloc(size);
    if (!_fpc_common_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    if (fpc_common_init(_fpc_common_buffer, &size)) {
        free(_fpc_common_buffer);
        _fpc_common_buffer = NULL;
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_common_t *c_instance = fpc_common_get_handle();
    size_t image_size = fpc_common_get_image_size();

    void *buffer = malloc(image_size);
    if (!buffer) {
        free(_fpc_common_buffer);
        _fpc_common_buffer = NULL;
        return -FPC_ERROR_ALLOC;
    }

    c_instance->image = fpc_common_init_default_image(buffer, image_size);

    c_instance->image_timestamp = (uint64_t) -1;

    c_instance->check_for_bad_pixels = 0;

    return 0;
}

static void fpc_ta_common_exit(void)
{

    free(_fpc_common_buffer);
    _fpc_common_buffer = NULL;
    free(fpc_common_get_handle()->image);
    fpc_common_get_handle()->image = NULL;
    LOG_LEAVE();

}

fpc_ta_module_t fpc_ta_common_module = {
    .init = fpc_ta_common_init,
    .exit = fpc_ta_common_exit,
    .handle_message = fpc_ta_common_command_handler,
    .key = TARGET_FPC_TA_COMMON,
};
