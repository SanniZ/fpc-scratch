/*
 * Hardware Identification Types
 *
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 * This file defines types for hardware identification. This information is typically
 * read from OTP memories on the separate dies or the hardware module at startup.
 */

#ifndef FPC_HW_IDENTIFICATION_TYPES_H
#define FPC_HW_IDENTIFICATION_TYPES_H

#include <stdint.h>

#define MAX_VENDOR_DATA_SIZE 20
#define MAX_LOT_ID_SIZE 6  /* Actual length can vary between 5-6 bytes */
#define MAX_PROD_TIMESTAMP_SIZE 10

#define INVALID_HARDWARE_ID 0xFFFF
#define INVALID_LOT_ID '\0'
#define INVALID_WAFTER_ID 0xFF
#define INVALID_WAFER_POSITION 0x7FFF
#define INVALID_PROD_TIMESTAMP '\0'

#define FPC_SENSOR_DIE_HARDWARE_ID_FIELD (1<<0)
#define FPC_SENSOR_DIE_LOT_ID_FIELD (1<<1)
#define FPC_SENSOR_DIE_WAFER_ID_FIELD (1<<2)
#define FPC_SENSOR_DIE_WAFER_POSITION_X_FIELD (1<<3)
#define FPC_SENSOR_DIE_WAFER_POSITION_Y_FIELD (1<<4)
#define FPC_SENSOR_DIE_PRODUCTION_TIMESTAMP_FIELD (1<<5)

#define FPC_COMPANION_DIE_HARDWARE_ID_FIELD (1<<0)
#define FPC_COMPANION_DIE_LOT_ID_FIELD (1<<1)

#define FPC_VENDOR_HW_DATA (1<<0)

/**
 * Sensor die related information.
 */
typedef struct fpc_sensor_die_info {
    uint16_t hardware_id;
    char lot_id[MAX_LOT_ID_SIZE+1]; /* Null terminated */
    uint8_t wafer_id;
    int16_t wafer_position_x;
    int16_t wafer_position_y;
    char production_timestamp[MAX_PROD_TIMESTAMP_SIZE+1]; /* Null terminated,
                                                                 Format yyyy-mm-dd */
    uint8_t valid_field;
} fpc_sensor_die_info_t;

/**
 * Companion die related information.
 */
typedef struct fpc_companion_die_info {
    uint16_t hardware_id;
    char lot_id[MAX_LOT_ID_SIZE+1]; /* Null terminated */
    uint8_t valid_field;
} fpc_companion_die_info_t;

/**
 * Vendor otp related information.
 */
typedef struct fpc_vendor_hw_info {
    uint8_t vendor_data[MAX_VENDOR_DATA_SIZE];
    uint8_t vendor_data_size;
    uint8_t valid_field;
} fpc_vendor_hw_info_t;

/**
 * OTP error information.
 */
typedef struct fpc_otp_error_info {
    uint32_t total_num_bit_errors;
    uint32_t max_num_bit_errors_in_byte; /* Highest number of bit errors that occured in a byte */
} fpc_otp_error_info_t;

/**
 * OTP related information.
 */
typedef struct fpc_hw_module_info {
    fpc_sensor_die_info_t sensor_die_info;
    fpc_companion_die_info_t companion_die_info;
    fpc_vendor_hw_info_t vendor_otp_info;
    fpc_otp_error_info_t otp_error_info;
    uint32_t product_type;
} fpc_hw_module_info_t;

#endif // FPC_HW_IDENTIFICATION_TYPES_H
