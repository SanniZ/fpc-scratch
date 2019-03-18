/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_M_UPGRADE_H
#define FPC_M_UPGRADE_H

#include <stdint.h>

typedef struct fpc_fingerprint_hal_device fpc_hal_common_t;

typedef struct fpc_m_upgrade fpc_m_upgrade_t;

struct fpc_m_upgrade {

    uint64_t (*pre_migrate_db)(fpc_m_upgrade_t *dev);

    int (*migrate_db)(fpc_m_upgrade_t* dev,
                      const uint8_t* hat,
                      uint32_t size_hat,
                      uint32_t gid, const char* store_path,
                      uint32_t** finger_list,
                      uint32_t* size_list);
};

fpc_m_upgrade_t* fpc_m_upgrade_new(fpc_hal_common_t* hal);
void fpc_m_upgrade_destroy(fpc_m_upgrade_t *dev);

#endif // FPC_M_UPGRADE_H

