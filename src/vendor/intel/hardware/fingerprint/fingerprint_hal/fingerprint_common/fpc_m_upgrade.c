/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <fpc_m_upgrade.h>

#include <fpc_log.h>
#include <fpc_tac.h>
#include <fpc_tee_hal.h>
#include <fpc_tac_m_upgrade.h>

#define MAX_FINGER_COUNT 5

typedef struct m_upgrade_module {
    fpc_m_upgrade_t upgrade;

    fpc_hal_common_t* hal;

    uint32_t ids[MAX_FINGER_COUNT];
} m_upgrade_module_t;


static int fpc_m_upgrade_load_legacy_db(fpc_m_upgrade_t* dev, uint32_t gid)
{
    LOGD("%s\n", __func__);
    fpc_hal_common_t* hal = ((m_upgrade_module_t*) dev)->hal;
    char filename[PATH_MAX];

    int size = snprintf(filename, PATH_MAX, "/data/fpcd/user%u.db", gid % 100);
    if (size >= PATH_MAX) {
        return -1;
    }

    int status = fpc_tac_load_user_db(hal->tee_handle, filename);

    if (status) {
        LOGE("%s load_user_db failed %i\n", __func__, status);
        return -1;
    }

    return 0;
}

static uint64_t pre_migrate_db(fpc_m_upgrade_t* dev)
{
    LOGD("%s\n", __func__);
    fpc_hal_common_t* hal = ((m_upgrade_module_t*) dev)->hal;

    pthread_mutex_lock(&hal->lock);
    fpc_worker_set_state(&hal->worker, STATE_IDLE);

    uint64_t challenge = 0;
    int status = fpc_tac_get_hw_auth_challenge(hal->tee_handle, &challenge);

    if (status) {
        LOGE("%s get_hw_auth_challenge failed %i\n", __func__, status);
        challenge = 0;
    }

    LOGD("%s challenge %" PRIu64 "\n", __func__, challenge);

    fpc_worker_set_state(&hal->worker, STATE_NONE);
    pthread_mutex_unlock(&hal->lock);

    return challenge;
}

static int migrate_db(fpc_m_upgrade_t* dev,
                      const uint8_t* hat,
                      uint32_t size_hat,
                      uint32_t gid,
                      const char* store_path,
                      uint32_t** finger_list,
                      uint32_t* size_list)
{
    LOGD("%s\n", __func__);
    m_upgrade_module_t* module = (m_upgrade_module_t*) dev;
    fpc_hal_common_t* hal = module->hal;

    pthread_mutex_lock(&hal->lock);
    fpc_worker_set_state(&hal->worker, STATE_IDLE);


    int status = fpc_tac_validate_auth_challenge(hal->tee_handle,
                                                hat, size_hat);

    if (status) {
        LOGE("%s validate_auth_challenge failed %i\n", __func__, status);
        goto out;
    }

    status = fpc_m_upgrade_load_legacy_db(dev, gid);

    if (status) {
        goto out;
    }

    status = fpc_tac_m_upgrade(hal->tee_handle, gid);

    if (status) {
        LOGE("%s fpc_tac_m_upgrade failed %i\n", __func__, status);
        goto out;
    }

    uint32_t indices[MAX_FINGER_COUNT];
    uint32_t indices_count;

    status = fpc_tac_get_template_count(hal->tee_handle, &indices_count);
    if (status) {
        LOGE("%s fpc_tac_get_template_count failed %i\n", __func__, status);
        goto out;
    }

    if (indices_count > MAX_FINGER_COUNT) {
        LOGE("%s indices_count (%u) > max\n", __func__, indices_count);
        status = -1;
        goto out;
    }

    status = fpc_tac_get_indices(hal->tee_handle, indices, &indices_count);
    if (status) {
        LOGE("%s fpc_tac_get_indices failed %i\n", __func__, status);
        goto out;
    }

    for (unsigned i = 0; i < indices_count; ++i) {
        uint32_t fid;
        status = fpc_tac_get_template_id_from_index(hal->tee_handle,
                                                    indices[i],
                                                    &fid);

        if (status) {
            LOGE("%s fpc_tac_get_template_id_from_index failed %i\n",
                 __func__, status);

            goto out;
        }

        module->ids[i] = fid;
    }

    char template_file[PATH_MAX];

    int length = snprintf(template_file, sizeof(template_file), "%s%s",
                          store_path, "/user.db");

    if (length < 0 || (unsigned) length >= sizeof(template_file)) {
        status -1;
        goto out;
    }

    status = fpc_tac_store_template_db(hal->tee_handle, template_file);

    if (status) {
        goto out;
    }

    *finger_list = module->ids;
    *size_list = indices_count;

out:
    fpc_worker_set_state(&hal->worker, STATE_NONE);
    pthread_mutex_unlock(&hal->lock);

    if (status) {
        return -1;
    }

    return 0;
}

fpc_m_upgrade_t* fpc_m_upgrade_new(fpc_hal_common_t* hal)
{
    m_upgrade_module_t* module = malloc(sizeof(m_upgrade_module_t));

    if (!module) {
        return NULL;
    }

    module->upgrade.pre_migrate_db = pre_migrate_db;
    module->upgrade.migrate_db = migrate_db;
    module->hal = hal;

    return (fpc_m_upgrade_t*) module;
}

void fpc_m_upgrade_destroy(fpc_m_upgrade_t *dev)
{
    free(dev);
}
