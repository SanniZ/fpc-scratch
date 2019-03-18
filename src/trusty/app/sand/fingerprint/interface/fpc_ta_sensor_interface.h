/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE
#define INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE

#include "fpc_ta_interface.h"
#include "fpc_hw_identification_types.h"

typedef struct {
    fpc_ta_cmd_header_t header;
    int32_t response;
    fpc_hw_module_info_t data;
} fpc_ta_sensor_otp_info_t;

typedef struct {
    fpc_ta_simple_command_t header;
    int32_t cac_result;
} fpc_ta_sensor_capture_info_t;

typedef struct {
    fpc_ta_simple_command_t header;
    uint8_t ctrl;
} fpc_ta_sensor_early_stop_ctrl_t;

typedef struct {
    fpc_ta_simple_command_t header;
    uint8_t value;
} fpc_ta_sensor_force_value_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_simple_command_t sensor;
    fpc_ta_simple_command_t is_otp_supported;
    fpc_ta_sensor_otp_info_t otp_info;
    fpc_ta_sensor_capture_info_t capture_info;
    fpc_ta_sensor_early_stop_ctrl_t early_stop_ctrl;
    fpc_ta_sensor_force_value_t get_force_value;
    fpc_ta_simple_command_t is_sensor_force_supported;
} fpc_ta_sensor_command_t;

typedef enum {
  FPC_TA_SENSOR_CHECK_FINGER_LOST_CMD,
  FPC_TA_SENSOR_FINGER_LOST_WAKEUP_SETUP_CMD,
  FPC_TA_SENSOR_WAKEUP_SETUP_CMD,
  FPC_TA_SENSOR_CAPTURE_IMAGE_CMD,
  FPC_TA_SENSOR_DEEP_SLEEP_CMD,
  FPC_TA_SENSOR_IS_OTP_SUPPORTED_CMD,
  FPC_TA_SENSOR_GET_OTP_INFO_CMD,
  FPC_TA_SENSOR_EARLY_STOP_CTRL_CMD,
  FPC_TA_SENSOR_GET_FORCE_VALUE,
  FPC_TA_SENSOR_IS_SENSOR_FORCE_SUPPORTED,
} fpc_ta_sensor_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_SENSOR_INTERFACE */
