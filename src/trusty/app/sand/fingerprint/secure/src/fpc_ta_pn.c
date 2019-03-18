/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <string.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_module.h"
#include "fpc_ta_pn_interface.h"
#include "fpc_ta_common.h"
#include "fpc_ta_sensor.h"
#include "fpc_ta_bio.h"
#include "fpc_ta_bio_internal.h"

#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_crypto.h"
#include "fpc_ta_pn.h"
#include "fpc_sensor_pn.h"
#include "fpc_sensor.h"
#include "fpc_result.h"
#include "fpc_ta_hw_auth.h"


#define UNENCRYPTED 0
#define ENCRYPTED   1

#ifdef FPC_CONFIG_HW_AUTH
#define PN_AUTH_CHALLENGE_LIFETIME (HW_AUTH_CHALLENGE_LIFETIME)

typedef struct {
    uint64_t outgoing_challenge;
    uint64_t challenge_timestamp;
    fpc_hw_auth_token_t incoming_token;
} pn_security_t;

static pn_security_t pn_security = {
    .outgoing_challenge = 0,
    .challenge_timestamp = (uint64_t)-1,
};

static int pn_get_challenge(uint64_t *challenge)
{
    LOG_ENTER();

    int status;
    int retries = 5;

    *challenge = 0;
    pn_security.outgoing_challenge = 0;

    do {
        status = fpc_secure_random((uint8_t*) challenge, sizeof(uint64_t));
    } while (*challenge == 0 && retries-- > 0);

    if (status) {
        LOGE("%s: fpc_secure_random returned error: %d",
              __func__, status);
        return status;
    }
    if (*challenge == 0) {
        status = -FPC_ERROR_NOT_INITIALIZED;
        LOGE("%s: got value 0 from fpc_secure_random: %d",
              __func__, status);
        return status;
    }

    pn_security.challenge_timestamp = fpc_get_uptime();
    pn_security.outgoing_challenge = *challenge;

    return status;
}

static int pn_validate_incoming_token()
{
    LOG_ENTER();

    if (pn_security.outgoing_challenge == 0) {
        return -FPC_ERROR_INPUT;
    }

    if (pn_security.incoming_token.challenge != pn_security.outgoing_challenge) {
        LOGE("%s challenge is not identical", __func__);
        return -FPC_ERROR_INPUT;
    }

    uint64_t challenge_age = fpc_get_uptime() - pn_security.challenge_timestamp;

    LOGD("%s challenge_age %llu", __func__, challenge_age);

    if (challenge_age > PN_AUTH_CHALLENGE_LIFETIME) {
        LOGE("%s challenge expired", __func__);
        return -FPC_ERROR_TIMEDOUT;
    }

    return fpc_check_token_integrity(&pn_security.incoming_token);
}

static int pn_authorize(const uint8_t* token_buffer, uint32_t size_token)
{
    LOG_ENTER();

    if (size_token < sizeof(fpc_hw_auth_token_t)) {
        LOGE("%s buffer size too small for token", __func__);
        return -FPC_ERROR_INPUT;
    }

    memcpy(&pn_security.incoming_token, token_buffer,
           sizeof(fpc_hw_auth_token_t));

    return pn_validate_incoming_token();
}
#endif  /* FPC_CONFIG_HW_AUTH */

static int fpc_ta_pn_get_size(uint32_t *size, int encrypted)
{
    fpc_sensor_pn_get_size(size);
    if (*size == 0) {
        return -FPC_PN_FAILED;
    }

    if (encrypted) {
        *size += FPC_PN_MAGIC_LEN;
        *size = fpc_get_wrapped_size(*size);
    }

    return FPC_PN_OK;
}

static int fpc_ta_pn_calibrate_finger(int32_t *image_decision,
                                      int32_t *image_quality,
                                      int32_t *pn_quality,
                                      int32_t *progress)
{
    LOG_ENTER();

    fpc_ta_common_t *ta_common = fpc_common_get_handle();
    struct fpc_bio  *bio       = fpc_bio_get_handle();
    fpc_algo_pn_add_image_data_t pn_add_image_data;
    *image_decision = 0;
    *image_quality  = 0;

    if (fpc_sensor_communication_start()) {
        LOGE("<--%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }

    int ret = fpc_device_capture_pn_image(fpc_sensor_get_handle(),
                                          ta_common->image);

    if (ret == FPC_RESULT_FINGER_LOST) {
        ret = -FPC_PN_RETRY_CALIBRATION;
        goto out;
    }

    if (ret != FPC_RESULT_OK) {
        ret = -FPC_PN_FAILED;
        goto out;
    }

    ret = fpc_algo_pn_calibrate_finger(bio->algo_context,
                                       ta_common->image,
                                       &pn_add_image_data);
    if (ret != FPC_RESULT_OK) {
        ret = -FPC_PN_FAILED;
        goto out;
    }

    *image_decision = pn_add_image_data.image_decision;
    *image_quality  = pn_add_image_data.image_quality;
    *pn_quality     = pn_add_image_data.pn_quality;
    *progress       = pn_add_image_data.progress;

    if (pn_add_image_data.progress < 100) {
        ret = -FPC_PN_RETRY_CALIBRATION;
        goto out;
    }

    ret = FPC_PN_OK;

out:
    if (fpc_sensor_communication_stop()) {
        ret = -FPC_PN_FAILED;
    }

    LOG_LEAVE_TRACE(ret);

    return ret;
}

static int fpc_ta_pn_load(uint8_t *encrypted_buffer, const uint32_t encrypted_size)
{
    int error = 0;

    uint32_t  unencrypted_size;
    uint8_t  *unencrypted_buffer = NULL;

    uint32_t  crypto_size;
    uint32_t *pn_magic;

    LOG_ENTER();

    fpc_ta_common_t *c_instance = fpc_common_get_handle();

    if (!encrypted_buffer || !encrypted_size) {
        LOGE("<--%s missing buffer", __func__);
        return -FPC_ERROR_ALLOC;
    }

    error = fpc_ta_pn_get_size(&unencrypted_size, UNENCRYPTED);
    if (error != FPC_RESULT_OK) {
        return error;
    }
    unencrypted_size += FPC_PN_MAGIC_LEN;

    crypto_size = unencrypted_size;
    error = fpc_unwrap_crypto(encrypted_buffer, encrypted_size,
                              &unencrypted_buffer, &crypto_size);
    if (error || crypto_size != unencrypted_size) {
        LOGE("<--%s crypto error: %d. Actual=%u expected=%u",
             __func__, error, crypto_size, unencrypted_size);
        goto out;
    }

    pn_magic = (uint32_t*)(unencrypted_buffer + unencrypted_size - FPC_PN_MAGIC_LEN);
    if (*pn_magic != FPC_PN_MAGIC) {
        LOGE("<--%s PN magic error", __func__);
        goto out;
    }

    if (fpc_sensor_load_pn(unencrypted_buffer, unencrypted_size,
                           c_instance->image) == FPC_RESULT_OK) {
        error = FPC_PN_OK;
    } else {
        error = -FPC_PN_FAILED;
    }

out:
    LOG_LEAVE();

    return error;
}

static int fpc_ta_pn_calibrate_finger_end(uint8_t        *encrypted_buffer,
                                          const uint32_t  size)
{
    LOGD("-->%s size=%u", __func__, size);

    int ret;
    uint32_t  encrypted_size;
    uint8_t  *unencrypted_buffer = NULL;
    uint32_t  unencrypted_size;

    fpc_bio_t       *bio       = fpc_bio_get_handle();
    fpc_ta_common_t *ta_common = fpc_common_get_handle();

#ifdef FPC_CONFIG_HW_AUTH
    ret = pn_validate_incoming_token();
    if (ret) {
        /* fpc_algo_pn_calibrate_finger_end() must always be called
         * for proper cleanup in BioLib. */
        (void)fpc_algo_pn_calibrate_finger_end(bio->algo_context, NULL);
        ret = -FPC_ERROR_TIMEDOUT;
        goto out;
    }
#endif

    ret = fpc_algo_pn_calibrate_finger_end(bio->algo_context,
                                           size ? ta_common->image : NULL);
    if (ret != FPC_RESULT_OK) {
        ret = -FPC_PN_FAILED;
        goto out;
    }

    /* In case of previous failure in fpc_ta_pn_calibrate_finger(),
     * 'encrypted_buffer' and 'size' should be NULL and 0
     * respectively. In that case only cleanup in BioLib will be
     * done (see above). */
    if (!encrypted_buffer || !size) {
        ret = FPC_PN_OK;
        goto out;
    }

    ret = fpc_ta_pn_get_size(&encrypted_size, ENCRYPTED);
    if (ret != FPC_RESULT_OK) {
        goto out;
    }

    if (encrypted_size > size) {
        LOGE("%s buffer too small, actual=%u expected=%u",
             __func__, size, encrypted_size);
        ret = -FPC_PN_FAILED;
        goto out;
    }

    ret = fpc_ta_pn_get_size(&unencrypted_size, UNENCRYPTED);
    if (ret != FPC_RESULT_OK) {
        goto out;
    }

    unencrypted_buffer = (uint8_t*)malloc(unencrypted_size + FPC_PN_MAGIC_LEN);
    if (!unencrypted_buffer) {
        ret = -FPC_PN_MEMORY;
        goto out;
    }


    ret = fpc_sensor_image_to_pn_image(ta_common->image,
                                       unencrypted_buffer,
                                       unencrypted_size);
    if (ret != FPC_RESULT_OK) {
        ret = -FPC_PN_FAILED;
    } else {
        uint32_t *pn_magic = (uint32_t*)(unencrypted_buffer + unencrypted_size);
        *pn_magic = FPC_PN_MAGIC;

        uint32_t crypto_size = encrypted_size;
        int32_t crypto_error =
            fpc_wrap_crypto(unencrypted_buffer, unencrypted_size + FPC_PN_MAGIC_LEN,
                            encrypted_buffer, &crypto_size);

        if (crypto_error || crypto_size != encrypted_size) {
            LOGE("<--%s crypto error (%d) or size mismatch, actual=%u expected=%u",
                 __func__, crypto_error, crypto_size, encrypted_size);
            ret = -FPC_PN_FAILED;
        } else {
            ret = FPC_PN_OK;
        }
    }

    free(unencrypted_buffer);

out:
    LOG_LEAVE_TRACE(ret);

    return ret;
}

#ifdef FPC_CONFIG_ENGINEERING
#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
static int fpc_ta_pn_calibrate(uint8_t *encrypted_buffer, const uint32_t size)
{
    int              ret = -FPC_PN_FAILED;
    int              sensor_return;
    uint32_t         encrypted_size;
    uint8_t         *unencrypted_buffer;
    uint32_t         unencrypted_size;

    LOGD("-->%s size=%u", __func__, size);

    ret = fpc_ta_pn_get_size(&encrypted_size, ENCRYPTED);
    if (ret != FPC_RESULT_OK) {
        goto out;
    }
    if (encrypted_size > size) {
        LOGE("<--%s buffer too small, actual=%u expected=%u",
             __func__, size, encrypted_size);
        return -FPC_PN_FAILED;
    }

    if (fpc_sensor_communication_start()) {
        LOGE("<--%s communication start failed.", __func__);
        return -FPC_ERROR_RESET_HARDWARE;
    }


    ret = fpc_ta_pn_get_size(&unencrypted_size, UNENCRYPTED);
    if (ret != FPC_RESULT_OK) {
        goto out;
    }
    unencrypted_buffer = (uint8_t*)malloc(unencrypted_size + FPC_PN_MAGIC_LEN);
    if (!unencrypted_buffer) {
        ret = -FPC_PN_MEMORY;
        goto out;
    }


    sensor_return = fpc_sensor_pn_calibrate(unencrypted_buffer, unencrypted_size);

    if (sensor_return == FPC_RESULT_OK) {

        uint32_t *pn_magic = (uint32_t*)(unencrypted_buffer + unencrypted_size);
        *pn_magic = FPC_PN_MAGIC;

        uint32_t crypto_size = encrypted_size;
        int32_t crypto_error =
            fpc_wrap_crypto(unencrypted_buffer, unencrypted_size + FPC_PN_MAGIC_LEN,
                            encrypted_buffer, &crypto_size);

        if (crypto_error || crypto_size != encrypted_size) {
            LOGE("<--%s crypto error (%d) or size mismatch, actual=%u expected=%u",
                 __func__, crypto_error, crypto_size, encrypted_size);
        } else {
            ret = FPC_PN_OK;
        }
    } else if (sensor_return == FPC_RESULT_FINGER_LOST) {
        ret = -FPC_PN_RETRY_CALIBRATION;
    } else {
        ret = -FPC_PN_FAILED;
    }

    free(unencrypted_buffer);

out:
    if (fpc_sensor_communication_stop()) {
        ret = -FPC_PN_FAILED;
    }

    LOG_LEAVE_TRACE(ret);

    return ret;
}
#endif  /* FPC_CONFIG_ALLOW_PN_CALIBRATE */

static int fpc_ta_pn_get_unencrypted_image(uint8_t *buffer, uint32_t size)
{
    LOG_ENTER();

    uint32_t unencrypted_size;
    int ret = fpc_ta_pn_get_size(&unencrypted_size, UNENCRYPTED);
    if (ret != FPC_RESULT_OK) {
        return ret;
    }
    if (unencrypted_size > size) {
        LOGE("<--%s buffer too small %u, expected %u",
             __func__, size, unencrypted_size);
        return -FPC_ERROR_ALLOC;
    }

    fpc_ta_common_t *ta_common = fpc_common_get_handle();
    ret = fpc_sensor_image_to_pn_image(ta_common->image, buffer, size);
    if (ret != FPC_RESULT_OK) {
        return -FPC_PN_FAILED;
    }

    return FPC_PN_OK;
}
#endif  /* FPC_CONFIG_ENGINEERING */

int fpc_ta_pn_command_handler(void* buffer, uint32_t size_buffer)
{
    LOG_ENTER();

    int ret = 0;

    fpc_ta_pn_command_t* command = shared_cast_to(fpc_ta_pn_command_t,
                                                  buffer, size_buffer);
    if (!command) {
        LOGE("%s, no command?", __func__);
        return -FPC_ERROR_INPUT;
    }

    switch (command->header.command) {
    case FPC_TA_PN_GET_SIZE:
        command->pn_get_size.response =
            fpc_ta_pn_get_size(&command->pn_get_size.size, ENCRYPTED);
        break;
    case FPC_TA_PN_LOAD_CMD:
        command->pn_load.response =
            fpc_ta_pn_load(
                command->pn_load.array,
                command->pn_load.size);
        break;
    case FPC_TA_PN_CALIBRATE_FINGER_CMD:
        command->pn_calibrate_finger.response =
            fpc_ta_pn_calibrate_finger(
                &command->pn_calibrate_finger.image_decision,
                &command->pn_calibrate_finger.image_quality,
                &command->pn_calibrate_finger.pn_quality,
                &command->pn_calibrate_finger.progress);
        break;
    case FPC_TA_PN_CALIBRATE_FINGER_END_CMD:
        command->pn_calibrate_finger_end.response =
            fpc_ta_pn_calibrate_finger_end(command->pn_calibrate_finger_end.array,
                                           command->pn_calibrate_finger_end.size);
        break;
#ifdef FPC_CONFIG_HW_AUTH
    case FPC_TA_PN_GET_CHALLENGE: {
        uint64_t challenge;
        command->pn_get_challenge.response = pn_get_challenge(&challenge);
        command->pn_get_challenge.challenge = challenge;
        break;
    }
    case FPC_TA_PN_AUTHORIZE:
        command->pn_authorize.response =
                pn_authorize(command->pn_authorize.array,
                             command->pn_authorize.size);
        break;
#endif    /* FPC_CONFIG_HW_AUTH */

#ifdef FPC_CONFIG_ENGINEERING
#ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
    case FPC_TA_PN_CALIBRATE_CMD:
        command->pn_calibrate.response =
            fpc_ta_pn_calibrate(command->pn_calibrate.array,
                                command->pn_calibrate.size);
        break;
#endif
    case FPC_TA_PN_GET_UNENCRYPTED_SIZE: {
        command->pn_get_size.response =
            fpc_ta_pn_get_size(&command->pn_get_size.size, UNENCRYPTED);
    } break;
    case FPC_TA_PN_GET_UNENCRYPTED_IMAGE:
        command->pn_unencrypted_image.response =
            fpc_ta_pn_get_unencrypted_image(command->pn_unencrypted_image.array,
                                            command->pn_unencrypted_image.size);
        break;
#endif  /* FPC_CONFIG_ENGINEERING */
    default:
        LOGE("%s unknown command %d", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return ret;
}

fpc_ta_module_t fpc_ta_pn_module = {
    .init = NULL,
    .exit = NULL,
    .handle_message = fpc_ta_pn_command_handler,
    .key = TARGET_FPC_TA_PN,
};
