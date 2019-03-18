/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdlib.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_tee.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"

#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "string.h"


static int bio_command(fpc_tee_bio_t* bio, int32_t command_id)
{
    fpc_tee_t* tee = &bio->tee;

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->header.command = command_id;
    command->header.target = TARGET_FPC_TA_BIO;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    if (0 > command->bio.response) {
        /* Grab the error before we continue */
        fpc_tee_get_error_log(tee);
    }

    return command->bio.response;
}

int fpc_tee_set_gid(fpc_tee_bio_t* bio, int32_t gid)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->bio.answer = gid;
    int status = bio_command(bio,
            FPC_TA_BIO_SET_ACTIVE_FINGERPRINT_SET_CMD);

    if (status) {
        return status;
    }

    return command->bio.response;
}

int fpc_tee_begin_enrol(fpc_tee_bio_t* bio)
{
    LOGD("%s", __func__);
    return bio_command(bio, FPC_TA_BIO_BEGIN_ENROL_CMD);
}

int fpc_tee_enrol(fpc_tee_bio_t* bio, uint32_t* remaining)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    int status = bio_command(bio, FPC_TA_BIO_ENROL_CMD);
    if (status < 0) {
        return status;
    }

    *remaining = command->bio.answer;
    return command->bio.response;
}

int fpc_tee_end_enrol(fpc_tee_bio_t* bio, uint32_t* id)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    int status = bio_command(bio, FPC_TA_BIO_END_ENROL_CMD);
    if (status) {
        return status;
    }

    *id = command->bio.answer;
    return command->bio.response;
}

int fpc_tee_identify(fpc_tee_bio_t* bio, uint32_t* id)
{
    LOGD("%s", __func__);

    int status = bio_command(bio, FPC_TA_BIO_IDENTIFY_CMD);
    if (status) {
        return status;
    }

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    *id = command->bio.answer;

    LOGD("%s: coverage: %d, quality: %d",
         __func__, command->identify.statistics.coverage, command->identify.statistics.quality);

    return command->bio.response;
}

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_get_identify_statistics(fpc_tee_bio_t* bio, fpc_ta_bio_identify_statistics_t* stat)
{
    LOGD("%s", __func__);
    int status = bio_command(bio, FPC_TA_BIO_GET_IDENTIFY_STATISTICS_CMD);
    if (status) {
        return status;
    }

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);

    stat->coverage = command->identify.statistics.coverage;
    stat->quality = command->identify.statistics.quality;
    stat->covered_zones = command->identify.statistics.covered_zones;
    stat->score = command->identify.statistics.score;
    stat->result = command->identify.statistics.result;

    return command->bio.response;
}
#endif

int fpc_tee_update_template(fpc_tee_bio_t* bio, uint32_t* update)
{
    LOGD("%s", __func__);
    int status = bio_command(bio, FPC_TA_BIO_UPDATE_TEMPLATE_CMD);

    if (status) {
        return status;
    }

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    *update = command->bio.answer;

    return command->bio.response;
}

int fpc_tee_get_finger_ids(fpc_tee_bio_t* bio, uint32_t* size, uint32_t* ids)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->bio.answer = *size;

    if (*size > MAX_NR_TEMPLATES) {
        return -FPC_ERROR_INPUT;
    }

    int status = bio_command(bio, FPC_TA_BIO_GET_FINGER_IDS_CMD);
    if (status) {
        return status;
    }

    *size = command->bio.answer;
    memcpy(ids, command->get_ids.ids, *size * sizeof(command->get_ids.ids[0]));

    return command->bio.response;
}

int fpc_tee_delete_template(fpc_tee_bio_t* bio, uint32_t id)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->bio.answer = id;
    int status = bio_command(bio, FPC_TA_BIO_DELETE_TEMPLATE_CMD);
    if (status) {
        return status;
    }

    return command->bio.response;
}

int fpc_tee_get_template_db_id(fpc_tee_bio_t* bio, uint64_t* id)
{
    LOGD("%s", __func__);

    int status = bio_command(bio, FPC_TA_BIO_GET_TEMPLATE_DB_ID_CMD);
    if (status) {
        return status;
    }
    fpc_ta_bio_get_db_command_t* command = (fpc_ta_bio_get_db_command_t*)
                                    bio->tee.shared_buffer->addr;
    *id = command->id;
    return command->simple.response;
}

int fpc_tee_load_empty_db(fpc_tee_bio_t* bio)
{
    LOGD("%s", __func__);
    return bio_command(bio, FPC_TA_BIO_LOAD_EMPTY_DB_CMD);
}


fpc_tee_bio_t* fpc_tee_bio_init(fpc_tee_t* tee)
{
    return (fpc_tee_bio_t*) tee;
}

void fpc_tee_bio_release(fpc_tee_bio_t* bio)
{
    (void) bio; // unused
    return;
}
