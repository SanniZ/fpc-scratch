/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE
#define INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE

#include "fpc_ta_interface.h"

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_byte_array_msg_t error_msg;
} fpc_ta_common_command_t;

typedef enum {
  FPC_TA_COMMON_GET_ERROR_LOG_CMD,
} fpc_ta_common_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_COMMON_INTERFACE */
