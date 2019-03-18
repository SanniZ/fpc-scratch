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
#include "fpc_ta_bio.h"
#include "fpc_ta_bio_internal.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_db.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_ta_common.h"
#include "fpc_result.h"

#ifdef FPC_CONFIG_TA_FS
#include "fpc_fs.h"
#endif
#include "fpc_crypto.h"
#include "fpc_ta_module.h"
#ifdef FPC_CONFIG_QC_AUTH
#include "fpc_ta_qc_auth.h"
#endif
#ifdef FPC_CONFIG_HW_AUTH
#include "fpc_ta_hw_auth.h"
#endif

#define INVALID_FINGERPRINT_SET_KEY 0xFFFFFFFF

fpc_bio_t g_fpc_trusted_app_instance;

fpc_bio_t* fpc_bio_get_handle()
{
    return &g_fpc_trusted_app_instance;
}

void fpc_match_result_reset(fpc_ta_match_result_t* result)
{
    LOG_ENTER();
    result->template_id = 0;
    result->timestamp = (uint64_t) -1;
    memset(&result->template_id256, 0, sizeof(id256_t));
}

static int fpc_result_to_error(int result)
{
    switch (result) {
        case FPC_RESULT_ERROR_MEMORY:
            return -FPC_ERROR_ALLOC;

        case FPC_RESULT_ERROR_PARAMETER:
            return -FPC_ERROR_INPUT;

        case FPC_RESULT_ERROR_TIMEDOUT:
            return -FPC_ERROR_TIMEDOUT;

        case FPC_RESULT_ERROR_SPI:
            return -FPC_ERROR_IO;

        case FPC_RESULT_ERROR_PN:
            return -FPC_ERROR_PN;

        case FPC_RESULT_ERROR_STATE:
            return -FPC_ERROR_NOT_INITIALIZED;

        case FPC_RESULT_ERROR_SENSOR:
            return -FPC_ERROR_SENSOR_BROKEN;

        case FPC_RESULT_ERROR_DEAD_PIXELS:
            return -FPC_ERROR_TOO_MANY_DEAD_PIXELS;

        case FPC_RESULT_ERROR_TEMPLATE_CORRUPTED:
            return -FPC_ERROR_TEMPLATE_CORRUPTED;

        case FPC_RESULT_ERROR_TEST_FAILED:
        case FPC_RESULT_ERROR_UNKNOWN:
        case FPC_RESULT_ERROR_NOT_SUPPORTED:
        case FPC_RESULT_ERROR_OTP:
        default:
            LOGE("%s: No conversion for error %d, returning -FPC_ERROR_INPUT", __func__, result);
            return -FPC_ERROR_INPUT;
    }
}

static int fpc_ta_bio_set_active_fingerprint_set(fpc_bio_t* bio,
                                      int32_t fingerprint_set_key)
{
    LOG_ENTER();
    bio->active_set_key = fingerprint_set_key;
    return 0;
}

static int fpc_ta_bio_qualify_image(void)
{
    fpc_ta_common_t *c_instance = fpc_common_get_handle();
    int status = fpc_algo_qualify_image(c_instance->image);
    LOG_ENTER();

    if (status == FPC_RESULT_BAD_QUALITY) {
        LOGE("%s: l returned error: %d", __func__, status);
        return FPC_CAPTURE_BAD_QUALITY;

    } else if (FAILED(status)) {
        LOGE("%s: qualify image returned error: %d", __func__, status);
        return -FPC_ERROR_IO;
    }
    return FPC_CAPTURE_OK;
}

static int fpc_ta_bio_begin_enrol(fpc_bio_t* bio)
{
    LOG_ENTER();
    int status;

    fpc_ta_common_t* common = fpc_common_get_handle();

    common->check_for_bad_pixels = 1;
#ifdef FPC_CONFIG_IDENTIFY_AT_ENROL
    bio->size_list = FPC_DB_FINGERPRINTS_IN_SET;
    status = fpc_db_get_all_templates(bio->user_db, bio->list, &bio->size_list);
    if (status) {
        return status;
    }

    status = fpc_algo_begin_identify(
        bio->algo_context,
        &bio->list[0],
        bio->size_list);
    if (status) {
        LOGE("%s: fpc_algo_begin_identify returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

#endif

    status = fpc_algo_begin_enroll(bio->algo_context);
    if (status) {
        LOGE("%s: fpc_algo_begin_enroll returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

    return 0;
}

static int fpc_ta_bio_enrol(fpc_bio_t* bio, uint32_t *remaining)
{
    LOG_ENTER();
    if (!bio || !remaining) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }

    fpc_ta_common_t *c_instance = fpc_common_get_handle();

    memset(&bio->enroll_data, 0, sizeof(bio->enroll_data));

    int status = fpc_algo_enroll(
        bio->algo_context,
        c_instance->image,
        &bio->enroll_data);

    if (FAILED(status)) {
        LOGE("%s: fpc_algo_enroll returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

    *remaining = bio->enroll_data.remaining_touches;

    switch (bio->enroll_data.result) {
    case FPC_ALGO_ENROLL_SUCCESS:
        return !bio->enroll_data.remaining_touches ?
            FPC_ENROL_COMPLETED : FPC_ENROL_PROGRESS;
    case FPC_ALGO_ENROLL_IMAGE_TOO_SIMILAR:
        return !bio->enroll_data.remaining_touches ?
            FPC_ENROL_COMPLETED : FPC_ENROL_IMAGE_TOO_SIMILAR;
    case FPC_ALGO_ENROLL_FAIL_ALREADY_ENROLLED:
        return FPC_ENROL_FAILED_ALREADY_ENROLED;
    case FPC_ALGO_ENROLL_TOO_MANY_FAILED_ATTEMPTS:
        return FPC_ENROL_FAILED_COULD_NOT_COMPLETE;
    case FPC_ALGO_ENROLL_FAIL_LOW_COVERAGE:
        return FPC_ENROL_IMAGE_LOW_COVERAGE;
    default:
        return FPC_ENROL_IMAGE_LOW_QUALITY;
    }
}

static int fpc_ta_bio_end_enrol(fpc_bio_t* bio, uint32_t* id)
{
    LOG_ENTER();
    int status;
    fpc_algo_template_t tpl;
    int run_end_enrol = 1;
    int tpl_ownership_transferred = 0;
    uint32_t index;

    if (!bio || !id) {
        LOGE("%s: unexpected Null pointer", __func__);
        status = -FPC_ERROR_CONFIG;
        goto out;
    }

    tpl.size = bio->enroll_data.enrolled_template_size;
    tpl.tpl = malloc(bio->enroll_data.enrolled_template_size);

    if (tpl.tpl == NULL) {
        LOGE("No memory for template");
        status = -FPC_ERROR_ALLOC;
        goto out;
    }

    run_end_enrol = 0;

    status = fpc_algo_end_enroll(bio->algo_context, &tpl);
    if (FAILED(status)) {
        LOGE("fpc_algo_end_enroll failed with error %d", status);
        status = fpc_result_to_error(status);
        goto out;
    }

    uint64_t user_id;
    if (!fpc_enrollment_allowed(bio, &user_id)) {
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    // Ownership of the template memory will be transferred to the database
    status = fpc_db_add_fingerprint(bio->user_db,
      bio->active_set_key, user_id, &tpl, &index);

    if (status != 0) {
        LOGE("fpc_db_add_fingerprint failed with error %d", status);
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    tpl_ownership_transferred = 1;

    id256_t id256;
    status = fpc_db_get_fingerprint_id(bio->user_db,
                                       bio->active_set_key,
                                       index, &id256);
    if (status) {
        status = -FPC_ERROR_INPUT;
        goto out;
    }

    fpc_db_trim_id_32(&id256, id);

out:
    if (run_end_enrol) {
        (void) fpc_algo_end_enroll(bio->algo_context, &tpl);
    }

#ifdef FPC_CONFIG_IDENTIFY_AT_ENROL
    fpc_algo_end_identify(bio->algo_context);
#endif
    fpc_ta_common_t* c_instance = fpc_common_get_handle();
    c_instance->check_for_bad_pixels = 1;

    if (!tpl_ownership_transferred) {
        free(tpl.tpl);
    }

    return status;
}

static uint8_t fpc_ta_liveness_enabled(void) {
#if defined(FPC_CONFIG_LIVENESS_DETECTION_ENABLED)
    return 1;
#elif defined(FPC_CONFIG_LIVENESS_DETECTION_APP_ONLY) && defined(FPC_CONFIG_HW_AUTH)
    return fpc_ta_hw_has_challenge();
#elif defined(FPC_CONFIG_LIVENESS_DETECTION_APP_ONLY) && defined(FPC_CONFIG_QC_AUTH)
    return fpc_ta_qc_auth_is_verify() == 1;
#else
    return 0;
#endif
}

#ifdef FPC_CONFIG_ENGINEERING
static int fpc_ta_bio_get_identify_statistics(fpc_bio_t* bio, fpc_ta_bio_identify_statistics_t* stat)
{
    LOG_ENTER();

    if (!bio || !stat) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }

    memset(stat, 0, sizeof(fpc_ta_bio_identify_statistics_t));

    stat->coverage = bio->ident_data.coverage;
    stat->quality = bio->ident_data.quality;
    stat->covered_zones = bio->ident_data.covered_zones;
    stat->score = bio->ident_data.score;
    stat->result = bio->ident_data.result;

    return 0;
}
#endif

static int fpc_ta_bio_identify(fpc_bio_t* bio,
                               uint32_t* id,
                               fpc_ta_bio_identify_statistics_t* stat)
{
    int status;
    LOG_ENTER();
    fpc_ta_common_t *c_instance;
    uint8_t liveness_enabled = fpc_ta_liveness_enabled();

    bio->size_list = FPC_DB_FINGERPRINTS_IN_SET;

    if (!bio || !id || !stat) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }
    if (bio->user_db == NULL) {
        LOGE("%s user_db = NULL, not cool", __func__);
        return -FPC_ERROR_INPUT;
    }

    *id = 0;
    stat->coverage = 0;
    stat->quality  = 0;
    stat->covered_zones = 0;
    stat->result = 0;
    stat->score = 0;
    stat->index = 0;

    status = fpc_ta_bio_qualify_image();
    if (status != FPC_CAPTURE_OK) {
        return status;
    }

    memset(&bio->ident_data, 0, sizeof(fpc_algo_identify_data_t));

    status = fpc_db_get_all_templates(bio->user_db, &bio->list[0], &bio->size_list);

    LOGD("%s got %u templates", __func__, bio->size_list);

    if (status) {
        return status;
    }

    if (bio->size_list == 0) {
        LOGE("no templates to play with");
        return -FPC_ERROR_NOENTITY;
    }

    status = fpc_algo_begin_identify(
        bio->algo_context,
        &bio->list[0],
        bio->size_list);
    if (FAILED(status)) {
        LOGE("%s: fpc_algo_begin_identify returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }


    c_instance = fpc_common_get_handle();
    c_instance->check_for_bad_pixels = 1;

    status = fpc_algo_identify(
        bio->algo_context,
        c_instance->image,
        &bio->ident_data,
        liveness_enabled);

    if (FAILED(status)) {
        LOGE("%s: fpc_algo_identify returned error: %d", __func__, status);
        status = fpc_result_to_error(status);
        goto err;
    }

    stat->coverage      = bio->ident_data.coverage;
    stat->quality       = bio->ident_data.quality;
    stat->covered_zones = bio->ident_data.covered_zones;
    stat->score         = bio->ident_data.score;
    stat->result        = bio->ident_data.result;

    if (bio->ident_data.result == FPC_ALGO_IDENTIFY_MATCH) {
        uint32_t db_index;
        status = fpc_db_get_index_of_template(bio->user_db,
                                       &bio->list[0] + bio->ident_data.index, &db_index);

        if (FAILED(status)) {
            goto err;
        }

        stat->index = db_index;

        id256_t id256;
        status = fpc_db_get_fingerprint_id(bio->user_db,
                                             bio->active_set_key,
                                             db_index, &id256);
        if (FAILED(status)) {
            goto err;
        }

        uint32_t template_id;
        fpc_db_trim_id_32(&id256, &template_id);
        bio->match_result.template_id = template_id;
        memcpy(&bio->match_result.template_id256, &id256, sizeof(id256_t));

        bio->match_result.timestamp = c_instance->image_timestamp;
        *id = template_id;
        fpc_db_set_template_first(bio->user_db, bio->ident_data.index);
    }

    return 0;
err:
    (void)fpc_algo_identify_update(bio->algo_context, &bio->ident_data);

    return status;
}

static int fpc_ta_bio_update_template(fpc_bio_t* bio, uint32_t* did_update)
{
    int status;
    LOG_ENTER();

    status = fpc_algo_identify_update(bio->algo_context, &bio->ident_data);
    if (status) {
        LOGE("%s: fpc_algo_identify_update returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

    if (bio->ident_data.update_result == FPC_ALGO_UPDATED_TEMPLATE) {
        LOGD("%s nbr templates updated %u", __func__, bio->ident_data.num_updated_templates);
        fpc_algo_template_t* updated_templates_list[FPC_ALGO_MAX_TEMPLATE_HANDLES];

        for (uint32_t i = 0; i < bio->ident_data.num_updated_templates; i++) {
            uint32_t db_index;
            status = fpc_db_get_index_of_template(bio->user_db,
                    &bio->list[0] + bio->ident_data.updated_template_indices[i],
                    &db_index);
            if (status) {
                return status;
            }

            updated_templates_list[i] = NULL;
            status = fpc_db_get_template(bio->user_db,
                                         bio->active_set_key,
                                         db_index,
                                         &updated_templates_list[i]);
            if (status) {
                return status;
            }
            if (!updated_templates_list[i]) {
                LOGE("%s: fpc_db_get_template returned Null pointer", __func__);
                return -FPC_ERROR_NOENTITY;
            }

            uint8_t* tpl_item = (uint8_t *) malloc(bio->ident_data.updated_template_sizes[i]);
            uint8_t* temp_tpl = updated_templates_list[i]->tpl;
            if (tpl_item == NULL) {
                LOGE("%s: failed to realloc template buffer, keeping old template", __func__);
                return -FPC_ERROR_ALLOC;
            }

            updated_templates_list[i]->size = bio->ident_data.updated_template_sizes[i];
            updated_templates_list[i]->tpl = tpl_item;

            free(temp_tpl);
        }
        status = fpc_algo_update_templates(bio->algo_context,
                                           &updated_templates_list[0],
                                           bio->ident_data.updated_template_indices,
                                           bio->ident_data.num_updated_templates);
        if (status) {
            LOGE("%s: fpc_algo_update_templates returned error: %d", __func__, status);
            return fpc_result_to_error(status);
        }
        *did_update = 1;
    } else {
        *did_update = 0;
    }

    status = fpc_algo_end_identify(bio->algo_context);
    if (status) {
        LOGE("%s: fpc_algo_end_identify returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

    return 0;
}

static int fpc_ta_bio_delete_template(fpc_bio_t* bio, uint32_t id)
{
    LOG_ENTER();
    uint32_t index;
    if (!bio || !id) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }
    int status = fpc_db_get_index_of_id32(bio->user_db, id, &index);
    if (FAILED(status)) {
        return status;
    }

    return fpc_db_delete_fingerprint(bio->user_db, bio->active_set_key, index);
}

static int fpc_ta_bio_load_empty_db(fpc_bio_t* bio)
{
    LOG_ENTER();
    if (!bio) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }
    fpc_db_destroy(&bio->user_db);
    return fpc_db_create(&bio->user_db, NULL, 0);
}

static int fpc_ta_bio_get_template_db_id(fpc_bio_t* bio, uint64_t *id)
{
    LOG_ENTER();
    if (!bio) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }
    return fpc_db_get_database_id(bio->user_db, id);
}

static int fpc_ta_bio_get_template_ids(fpc_bio_t* bio,
                            uint32_t* size, uint32_t* ids)
{
    LOG_ENTER();
    uint32_t indices_count = FPC_DB_FINGERPRINTS_IN_SET;
    uint32_t indices[FPC_DB_FINGERPRINTS_IN_SET];

    if (!bio) {
        LOGE("%s: unexpected Null pointer", __func__);
        return -FPC_ERROR_CONFIG;
    }
    int status = fpc_db_get_indices(bio->user_db,
                                    bio->active_set_key,
                                    indices, indices_count, &indices_count);

    if (FAILED(status)) {
        return status;
    }

    if (indices_count > *size) {
        return -FPC_ERROR_INPUT;
    }

    for (unsigned i = 0; i < indices_count; ++i) {
        id256_t large_tid;
        status = fpc_db_get_fingerprint_id(bio->user_db,
                             bio->active_set_key, indices[i], &large_tid);

        if (FAILED(status)) {
            return status;
        }

        fpc_db_trim_id_32(&large_tid, ids + i);
    }

    *size = indices_count;

    return 0;
}

static int fpc_ta_bio_command_handler(void* buffer, uint32_t size_buffer)
{
    LOG_ENTER();
    fpc_ta_bio_command_t* command = shared_cast_to(fpc_ta_bio_command_t,
                                               buffer, size_buffer);
    if (!command) {
        LOGE("%s, No command? Buffer:%p, Size:%x", __func__,
            buffer,
            size_buffer);
        return -FPC_ERROR_INPUT;
    }

    fpc_bio_t* bio = fpc_bio_get_handle();
    int32_t response = -FPC_ERROR_INPUT;
    uint32_t answer = command->bio.answer;

    switch (command->header.command) {
    case FPC_TA_BIO_SET_ACTIVE_FINGERPRINT_SET_CMD:
        response = fpc_ta_bio_set_active_fingerprint_set(bio, answer);
        break;
    case FPC_TA_BIO_BEGIN_ENROL_CMD:
        response = fpc_ta_bio_begin_enrol(bio);
        break;
    case FPC_TA_BIO_ENROL_CMD:
        response = fpc_ta_bio_enrol(bio, &answer);
        break;
    case FPC_TA_BIO_END_ENROL_CMD:
        response = fpc_ta_bio_end_enrol(bio, &answer);
        break;
    case FPC_TA_BIO_IDENTIFY_CMD:
        response = fpc_ta_bio_identify(bio, &answer, &command->identify.statistics);
        break;
    case FPC_TA_BIO_UPDATE_TEMPLATE_CMD:
        response = fpc_ta_bio_update_template(bio, &answer);
        break;
    case FPC_TA_BIO_LOAD_EMPTY_DB_CMD:
        response = fpc_ta_bio_load_empty_db(bio);
        break;
    case FPC_TA_BIO_GET_FINGER_IDS_CMD:
        response = fpc_ta_bio_get_template_ids(bio,
                                    &answer,
                                    command->get_ids.ids);
        break;
    case FPC_TA_BIO_DELETE_TEMPLATE_CMD:
        response = fpc_ta_bio_delete_template(bio, answer);
        break;

    case FPC_TA_BIO_GET_TEMPLATE_DB_ID_CMD:
        response = fpc_ta_bio_get_template_db_id(bio, &command->db_id.id);
        break;
#ifdef FPC_CONFIG_ENGINEERING
    case FPC_TA_BIO_GET_IDENTIFY_STATISTICS_CMD:
        response = fpc_ta_bio_get_identify_statistics(bio, &command->identify.statistics);
        break;
#endif
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    command->bio.response = response;
    command->bio.answer = answer;
    return 0;
}

static int fpc_ta_bio_init(void)
{
    LOG_ENTER();

    fpc_bio_t* bio = &g_fpc_trusted_app_instance;
    memset(bio, 0, sizeof(fpc_bio_t));

    bio->active_set_key = INVALID_FINGERPRINT_SET_KEY;

    int status = fpc_db_create(&bio->user_db, NULL, 0);
    if (status) {
        LOGE("%s fpc_db_create failed", __func__);
        return -FPC_ERROR_ALLOC;
    }

    uint32_t context_size;
    status = fpc_algo_load_configuration(NULL, &context_size);

    if(status == FPC_RESULT_ERROR_MEMORY) {
        bio->algo_context = (algo_context_t*) malloc(context_size);
    }

    status = fpc_algo_load_configuration(bio->algo_context, &context_size);
    if (FAILED(status)) {
        LOGE("%s: fpc_algo_load_configuration returned error: %d", __func__, status);
        return fpc_result_to_error(status);
    }

    fpc_ta_common_t* common = fpc_common_get_handle();
    common->check_for_bad_pixels = 1;

    return 0;
}

static void fpc_ta_bio_exit(void)
{
    LOG_ENTER();

    fpc_bio_t* bio = &g_fpc_trusted_app_instance;

    fpc_algo_cleanup(bio->algo_context);
    free(bio->algo_context);

    int status = fpc_db_destroy(&bio->user_db);
    if (status) {
        LOGE("%s: fpc_db_destroy returned error: %d", __func__, status);
    }

}

fpc_ta_module_t fpc_ta_bio_module = {
    .init = fpc_ta_bio_init,
    .exit = fpc_ta_bio_exit,
    .handle_message = fpc_ta_bio_command_handler,
    .key = TARGET_FPC_TA_BIO,
};
