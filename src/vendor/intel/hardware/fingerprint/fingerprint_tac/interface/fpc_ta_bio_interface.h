/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_BIO_INTERFACE
#define INCLUSION_GUARD_FPC_TA_BIO_INTERFACE

#include "fpc_ta_interface.h"

#define MAX_NR_TEMPLATES 5

#define FPC_TA_BIO_DB_RDONLY 0
#define FPC_TA_BIO_DB_WRONLY 1

typedef struct {
    fpc_ta_cmd_header_t cmd;
    int32_t response;
    uint32_t answer;
} fpc_ta_bio_simple_command_t;

typedef struct {
    int32_t coverage;
    int32_t quality;
    int32_t covered_zones;
    uint32_t result;
    uint32_t score;
    uint32_t index;
} fpc_ta_bio_identify_statistics_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    fpc_ta_bio_identify_statistics_t statistics;
} fpc_ta_bio_identify_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    uint32_t ids[MAX_NR_TEMPLATES];
} fpc_ta_bio_get_ids_command_t;

typedef struct {
    fpc_ta_simple_command_t simple;
    uint64_t id;
} fpc_ta_bio_get_db_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    uint32_t mode;
    uint32_t size;
} fpc_ta_bio_db_open_command_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_bio_simple_command_t bio;
    fpc_ta_byte_array_msg_t store_db;
    fpc_ta_byte_array_msg_t load_db;
    fpc_ta_bio_db_open_command_t db_open;
    fpc_ta_simple_command_t db_close;
    fpc_ta_byte_array_msg_t db_read;
    fpc_ta_byte_array_msg_t db_write;
    fpc_ta_byte_array_msg_t get_db_size;
    fpc_ta_bio_get_ids_command_t get_ids;
    fpc_ta_bio_identify_command_t identify;
    fpc_ta_bio_get_db_command_t db_id;
} fpc_ta_bio_command_t;


typedef enum {
  FPC_TA_BIO_BEGIN_ENROL_CMD,
  FPC_TA_BIO_ENROL_CMD,
  FPC_TA_BIO_END_ENROL_CMD,
  FPC_TA_BIO_IDENTIFY_CMD,
  FPC_TA_BIO_UPDATE_TEMPLATE_CMD,
  FPC_TA_BIO_GET_DB_SIZE_CMD,
  FPC_TA_BIO_LOAD_EMPTY_DB_CMD,
  FPC_TA_BIO_GET_FINGER_IDS_CMD,
  FPC_TA_BIO_DELETE_TEMPLATE_CMD,
  FPC_TA_BIO_SET_ACTIVE_FINGERPRINT_SET_CMD,
  FPC_TA_BIO_GET_TEMPLATE_DB_ID_CMD,
  FPC_TA_BIO_LOAD_DB_CMD,
  FPC_TA_BIO_STORE_DB_CMD,
  FPC_TA_BIO_DB_OPEN_CMD,
  FPC_TA_BIO_DB_CLOSE_CMD,
  FPC_TA_BIO_DB_READ_CMD,
  FPC_TA_BIO_DB_WRITE_CMD,
  FPC_TA_BIO_GET_IDENTIFY_STATISTICS_CMD,
} fpc_ta_bio_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_BIO_INTERFACE */

