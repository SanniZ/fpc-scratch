/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef INCLUSION_GUARD_FPC_TA_PN_INTERFACE
#define INCLUSION_GUARD_FPC_TA_PN_INTERFACE

#include "fpc_ta_interface.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    int32_t image_decision;
    int32_t image_quality;
    int32_t pn_quality;
    int32_t progress;
} fpc_ta_pn_calibrate_finger_t;

typedef struct {
    fpc_ta_cmd_header_t header;
    uint64_t challenge;
    int32_t response;
} fpc_ta_pn_challenge_cmd_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_byte_array_msg_t pn_calibrate;
    fpc_ta_size_msg_t pn_get_size;
    fpc_ta_byte_array_msg_t pn_load;
    fpc_ta_pn_calibrate_finger_t pn_calibrate_finger;
    fpc_ta_byte_array_msg_t pn_calibrate_finger_end;
    fpc_ta_pn_challenge_cmd_t pn_get_challenge;
    fpc_ta_byte_array_msg_t pn_authorize;
    fpc_ta_byte_array_msg_t pn_unencrypted_image;
} fpc_ta_pn_command_t;

typedef enum {
    FPC_TA_PN_CALIBRATE_CMD,
    FPC_TA_PN_GET_SIZE,                   /* Encrypted size */
    FPC_TA_PN_LOAD_CMD,
    FPC_TA_PN_CALIBRATE_FINGER_CMD,
    FPC_TA_PN_CALIBRATE_FINGER_END_CMD,
    FPC_TA_PN_GET_CHALLENGE,
    FPC_TA_PN_AUTHORIZE,
    FPC_TA_PN_GET_UNENCRYPTED_SIZE,
    FPC_TA_PN_GET_UNENCRYPTED_IMAGE,
} fpc_pn_cmd_t;

#endif
