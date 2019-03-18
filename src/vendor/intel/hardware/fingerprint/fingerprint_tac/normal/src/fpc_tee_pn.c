/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <string.h>
#include <hardware/hw_auth_token.h>

#include "fpc_tac.h"
#include "fpc_log.h"

#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee_sensor_internal.h"

#include "fpc_ta_pn_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_types.h"

int fpc_tee_pn_get_size(fpc_tee_sensor_t *sensor,
                        uint32_t         *image_size)
{
    LOGD("%s", __func__);

    fpc_tee_t *tee = sensor->tee;

    fpc_ta_pn_command_t *command =
        (fpc_ta_pn_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_PN_GET_SIZE;
    command->header.target = TARGET_FPC_TA_PN;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_get_size.response;
    *image_size = command->pn_get_size.size;

out:
    return status;
}

int fpc_tee_pn_load(fpc_tee_sensor_t*  sensor,
                    const void        *pn_buffer,
                    uint32_t           pn_size)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    size_t size = sizeof(fpc_ta_pn_command_t) + pn_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_pn_command_t* command =
        (fpc_ta_pn_command_t*) shared_buffer->addr;

    command->header.command = FPC_TA_PN_LOAD_CMD;
    command->header.target  = TARGET_FPC_TA_PN;
    command->pn_load.size   = pn_size;
    memcpy(command->pn_load.array, pn_buffer, pn_size);

    int status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_load.response;

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_pn_calibrate_finger(fpc_tee_sensor_t *sensor,
                                int32_t *image_decision,
                                int32_t *image_quality,
                                int32_t *pn_quality,
                                int32_t *progress)
{
    LOGD("%s", __func__);

    if (fpc_tee_sensor_cancelled(sensor)) {
        LOGD("%s PN calibration cancelled", __func__);
        return -FPC_ERROR_CANCELLED;
    }

    fpc_tee_t* tee = sensor->tee;

    size_t size = sizeof(fpc_ta_pn_command_t);

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_pn_command_t *command =
        (fpc_ta_pn_command_t*) shared_buffer->addr;

    command->header.command = FPC_TA_PN_CALIBRATE_FINGER_CMD;
    command->header.target  = TARGET_FPC_TA_PN;

    command->pn_calibrate_finger.image_decision = *image_decision;
    command->pn_calibrate_finger.image_quality  = *image_quality;
    command->pn_calibrate_finger.pn_quality     = *pn_quality;
    command->pn_calibrate_finger.progress       = *progress;

    int status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_calibrate_finger.response;

    if (status != -FPC_PN_FAILED) {
        *image_decision = command->pn_calibrate_finger.image_decision;
        *image_quality  = command->pn_calibrate_finger.image_quality;
        *pn_quality     = command->pn_calibrate_finger.pn_quality;
        *progress       = command->pn_calibrate_finger.progress;
    }

out:
    fpc_tac_free_shared(shared_buffer);

    LOGD("%s status=%d image_decision=%d image_quality=%d pn_quality=%d progress=%d",
         __func__, status,
         *image_decision, *image_quality, *pn_quality, *progress);

    return status;
}

int fpc_tee_pn_calibrate_finger_end(fpc_tee_sensor_t *sensor,
                                    void             *pn_buffer,
                                    const uint32_t    pn_size)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    size_t size = sizeof(fpc_ta_pn_command_t) + pn_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_pn_command_t *command =
        (fpc_ta_pn_command_t*)shared_buffer->addr;

    command->header.command               = FPC_TA_PN_CALIBRATE_FINGER_END_CMD;
    command->header.target                = TARGET_FPC_TA_PN;
    command->pn_calibrate_finger_end.size = pn_size;

    int status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_calibrate_finger_end.response;

    if (status == FPC_PN_OK && pn_buffer && pn_size) {
        memcpy(pn_buffer, command->pn_calibrate_finger_end.array, pn_size);
    }

out:
    fpc_tac_free_shared(shared_buffer);

    return status;
}

int fpc_tee_pn_get_challenge(fpc_tee_sensor_t *sensor, uint64_t *challenge)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    fpc_ta_pn_command_t *command =
            (fpc_ta_pn_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_PN_GET_CHALLENGE;
    command->header.target = TARGET_FPC_TA_PN;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        return status;
    }

    *challenge = command->pn_get_challenge.challenge;
    return command->pn_get_challenge.response;
}

int fpc_tee_pn_authorize(fpc_tee_sensor_t *sensor, const uint8_t* token,
        uint32_t size_token)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    const uint32_t size = sizeof(fpc_ta_pn_command_t) + size_token;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    int status;
    fpc_ta_pn_command_t* command = shared_buffer->addr;
    command->header.command = FPC_TA_PN_AUTHORIZE;
    command->header.target = TARGET_FPC_TA_PN;
    command->pn_authorize.size = size_token;
    memcpy(command->pn_authorize.array, token, size_token);

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_authorize.response;
out:
    fpc_tac_free_shared(shared_buffer);
    return status;
}

int fpc_tee_pn_calibrate(fpc_tee_sensor_t *sensor,
                         void             *image,
                         const uint32_t    image_size)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    size_t size = sizeof(fpc_ta_pn_command_t) + image_size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_pn_command_t *command =
        (fpc_ta_pn_command_t*)shared_buffer->addr;

    int status = fpc_irq_wakeup_enable(sensor->irq);
    if (status) {
        goto out;
    }

    command->header.command    = FPC_TA_PN_CALIBRATE_CMD;
    command->header.target     = TARGET_FPC_TA_PN;
    command->pn_calibrate.size = image_size;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_calibrate.response;

    if (status == FPC_PN_OK) {
        memcpy(image, command->pn_calibrate.array, image_size);
    }

out:
    fpc_irq_wakeup_disable(sensor->irq);
    fpc_tac_free_shared(shared_buffer);

    return status;
}

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_pn_get_unencrypted_size(fpc_tee_sensor_t *sensor,
                                    uint32_t *size)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    fpc_ta_pn_command_t* command =
        (fpc_ta_pn_command_t*) tee->shared_buffer->addr;

    command->header.command = FPC_TA_PN_GET_UNENCRYPTED_SIZE;
    command->header.target = TARGET_FPC_TA_PN;

    int status = fpc_tac_transfer(tee->tac, tee->shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_get_size.response;
    *size  = command->pn_get_size.size;

out:
    return status;
}

int fpc_tee_pn_get_unencrypted_image(fpc_tee_sensor_t *sensor,
                                     void             *image,
                                     const uint32_t    size)
{
    LOGD("%s", __func__);

    fpc_tee_t* tee = sensor->tee;

    size_t mem_size = sizeof(fpc_ta_pn_command_t) + size;

    fpc_tac_shared_mem_t *shared_buffer = fpc_tac_alloc_shared(tee->tac, mem_size);
    if (!shared_buffer) {
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_pn_command_t *command =
        (fpc_ta_pn_command_t*)shared_buffer->addr;

    int status = fpc_irq_wakeup_enable(sensor->irq);
    if (status) {
        goto out;
    }

    command->header.command            = FPC_TA_PN_GET_UNENCRYPTED_IMAGE;
    command->header.target             = TARGET_FPC_TA_PN;
    command->pn_unencrypted_image.size = size;

    status = fpc_tac_transfer(tee->tac, shared_buffer);
    if (status) {
        goto out;
    }

    status = command->pn_unencrypted_image.response;

    if (status == FPC_PN_OK) {
        memcpy(image,
               command->pn_unencrypted_image.array,
               command->pn_unencrypted_image.size);
    }

out:
    fpc_irq_wakeup_disable(sensor->irq);
    fpc_tac_free_shared(shared_buffer);

    return status;
}
#endif  /* FPC_CONFIG_ENGINEERING */
