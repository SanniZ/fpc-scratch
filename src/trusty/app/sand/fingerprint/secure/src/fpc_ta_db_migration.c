/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_HW_AUTHENTICATION
#error "HW AUTHENTICATION SUPPORT IS REQUIRED FOR DB MIGRATION"
#endif

#include <stdint.h>

#include <fpc_ta_bio.h>
#include <fpc_ta_internal.h>
#include <fpc_ta_db_migration.h>
#include <fpc_db.h>
#include "fpc_log.h"

int32_t fpc_ta_m_upgrade(fpc_bio_t* bio, int32_t set_id)
{
    LOG_ENTER();

    int status = 0;

    uint64_t secure_user_id;

    if (!fpc_enrollment_allowed(ta, &secure_user_id)) {
        LOGE("%s hw_auth_challenge not verified", __func__);
        return -FPC_ERROR_INPUT;
    }

    uint32_t index_count = FPC_DB_FINGERPRINTS_IN_SET;
    uint32_t indices[FPC_DB_FINGERPRINTS_IN_SET];

    status = fpc_db_get_indices(ta->user_db, set_id, indices, index_count,
                                &index_count);

    if (status) {
        LOGE("%s fpc_db_get_indices failed %i", __func__, status);
        return status;
    }

    if (index_count == 0) {
        LOGE("%s no data to migrate", __func__);
        return -FPC_ERROR_INPUT;
    }

    for (unsigned i = 0; i < index_count; ++i) {
        uint64_t user_id;
        status = fpc_db_get_secure_user_id(ta->user_db, set_id, indices[i],
                                           &user_id);

        if (status) {
            LOGE("%s fpc_db_get_secure_user_id failed %i", __func__, status);
            return status;
        }

        if (user_id != 0) {
            LOGE("%s failed, db already contains secure_user_id", __func__);
            return -FPC_ERROR_INPUT;
        }
    }

    for (unsigned i = 0; i < index_count; ++i) {
        status = fpc_db_set_secure_user_id(ta->user_db, set_id, indices[i],
                                           secure_user_id);

        if (status) {
            LOGE("%s fpc_db_set_secure_user_id failed %i", __func__, status);
            return status;
        }
    }

    return 0;
}
