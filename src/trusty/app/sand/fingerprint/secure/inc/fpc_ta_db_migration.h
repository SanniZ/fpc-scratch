/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_DB_MIGRATION_H
#define FPC_TA_DB_MIGRATION_H

#include <stdint.h>
#include <fpc_ta.h>

#define FINGERPRINT_TEE_M_UPGRADE_CMD_ID  ((fpc_ta_cmd_t) 1983)

typedef struct {
    uint32_t cmd_id;
    uint32_t gid;
}__attribute__ ((packed)) fpc_ta_m_upgrade_request_t;


typedef struct {
    int32_t status;
}__attribute__ ((packed)) fpc_ta_m_upgrade_response_t;

int32_t fpc_ta_m_upgrade(fpc_ta_t* ta, int32_t set_id);

#endif // FPC_TA_DB_MIGRATION_H

