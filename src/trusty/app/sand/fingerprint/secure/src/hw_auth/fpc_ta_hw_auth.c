/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#include <stddef.h>
#include <string.h>

#include "fpc_log.h"
#include "fpc_ta_bio_internal.h"
#include "fpc_types.h"
#include "fpc_ta_hw_auth.h"
#include "fpc_crypto.h"
#include "fpc_ta_hw_auth_interface.h"
#include "fpc_ta_hw_auth.h"
#include "fpc_ta_module.h"
#include "fpc_ta_bio.h"
#include "fpc_ta_targets.h"

static void swap_bytes(uint8_t* bytes, uint32_t size)
{
    uint8_t* first;
    uint8_t* last;
    for (first = bytes, last = bytes + size - 1;
         first < last; ++first, --last) {
        uint8_t temp = *first;
        *first = *last;
        *last = temp;
    }
}

typedef struct fpc_hw_auth_ta fpc_hw_auth_ta_t;

struct fpc_hw_auth_ta {
    fpc_bio_t* bio;
    uint64_t incoming_challenge;
    uint64_t outgoing_challenge;
    uint64_t outgoing_timestamp;
    fpc_hw_auth_token_t incoming_token;

    uint8_t hmac_key[32];
    int hmac_key_initialized;
};

static fpc_hw_auth_ta_t g_hw_auth_ta;

int fpc_check_token_integrity(fpc_hw_auth_token_t* token)
{
    fpc_hw_auth_ta_t* ta = &g_hw_auth_ta;

    if (token->version != 0) {
        return -FPC_ERROR_INPUT;
    }

    if (!ta->hmac_key_initialized) {
        LOGE("%s key for hmac validation not initialized", __func__);
        return -FPC_ERROR_INPUT;
    }

    uint8_t hmac[32];

    int status = fpc_hmac_sha256(
                  (uint8_t*) token, sizeof(*token) - sizeof(token->hmac),
                   ta->hmac_key, sizeof(ta->hmac_key), hmac);

    if (status) {
        LOGE("%s failed to create HMAC, error %d", __func__, status);
        return -FPC_ERROR_IO;
    }

    status = fpc_crypto_memcmp(hmac, token->hmac, sizeof(hmac));
    if (status) {
        LOGE("%s HMAC doesn't match", __func__);
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

static int validate_incoming_token(fpc_hw_auth_ta_t* ta)
{
    if (!ta->hmac_key_initialized) {
        LOGE("%s key for hmac validation not initialized", __func__);
        return -FPC_ERROR_INPUT;
    }

    if (ta->outgoing_challenge == 0) {
        return -FPC_ERROR_INPUT;
    }

    if (ta->incoming_token.challenge != ta->outgoing_challenge) {
        LOGE("%s challenge is not identical", __func__);
        return -FPC_ERROR_INPUT;
    }

    uint64_t challenge_age = fpc_get_uptime() - ta->outgoing_timestamp;

    LOGD("%s challenge_age %" PRIu64, __func__, challenge_age);

    if (challenge_age > HW_AUTH_CHALLENGE_LIFETIME) {
        LOGE("%s challenge expired", __func__);
        ta->outgoing_challenge = 0;
        return -FPC_ERROR_TIMEDOUT;
    }

    return fpc_check_token_integrity(&ta->incoming_token);
}

int fpc_enrollment_allowed(fpc_bio_t* bio, uint64_t* user)
{
    (void)bio; //Unused

    int status = validate_incoming_token(&g_hw_auth_ta);

    if (status) {
        return 0;
    }

    *user = g_hw_auth_ta.incoming_token.user_id;
    return 1;
}

static int get_enrol_challenge(fpc_hw_auth_ta_t* ta,
                                       uint64_t* challenge)
{
    int status;
    int retries = 5;

    *challenge = 0;
    ta->outgoing_challenge = 0;

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

    ta->outgoing_timestamp = fpc_get_uptime();
    ta->outgoing_challenge = *challenge;

    return status;
}

static int authorize_enrol(fpc_hw_auth_ta_t* ta,
                               const uint8_t* token_buffer, uint32_t size_token)
{
    if (size_token < sizeof(fpc_hw_auth_token_t)) {
        LOGE("%s buffer size too small for token", __func__);
        return -FPC_ERROR_INPUT;
    }

    memcpy(&ta->incoming_token, token_buffer, sizeof(fpc_hw_auth_token_t));

    return validate_incoming_token(ta);
}

static int set_auth_challenge(fpc_hw_auth_ta_t* hw_auth,
                                      uint64_t challenge)
{
    fpc_match_result_reset(&hw_auth->bio->match_result);
    hw_auth->incoming_challenge = challenge;
    return 0;
}

static int get_auth_result(fpc_hw_auth_ta_t* hw_auth,
                                    uint8_t* token_buffer, uint32_t size_token)
{
    int status;

    if (size_token < sizeof(fpc_hw_auth_token_t)) {
        LOGE("%s buffer size too small for token", __func__);
        return -FPC_ERROR_INPUT;
    }

    uint64_t secure_user_id = 0;
    uint32_t template_index;
    status = fpc_db_get_index_of_id32(hw_auth->bio->user_db,
                                 hw_auth->bio->match_result.template_id,
                                 &template_index);

    if (status) {
        return -FPC_ERROR_INPUT;
    }

    fpc_db_get_secure_user_id(hw_auth->bio->user_db,
                              hw_auth->bio->active_set_key,
                              template_index, &secure_user_id);

    fpc_hw_auth_token_t* token = (fpc_hw_auth_token_t*) token_buffer;


    uint64_t db_id;
    status = fpc_db_get_database_id(hw_auth->bio->user_db, &db_id);
    if (status) {
        return -FPC_ERROR_INPUT;
    }

    uint32_t authenticator_type = (1 << 1);
    uint64_t timestamp = hw_auth->bio->match_result.timestamp;
    swap_bytes((uint8_t*) &timestamp, sizeof(timestamp));
    swap_bytes((uint8_t*) &authenticator_type, sizeof(authenticator_type));

    token->version = 0;
    token->user_id = secure_user_id;
    token->authenticator_id = db_id;
    token->authenticator_type = authenticator_type;
    token->timestamp = timestamp;
    token->challenge = hw_auth->incoming_challenge;

    status = fpc_hmac_sha256((uint8_t*) token,
                 sizeof(*token) - sizeof(token->hmac),
                 hw_auth->hmac_key,
                 sizeof(hw_auth->hmac_key),
                 token->hmac);

    if (status != 0)
    {
        LOGE("%s failed to create HMAC, error %d", __func__, status);
        return -FPC_ERROR_IO;
    }

    return 0;
}

static int set_shared_key(fpc_hw_auth_ta_t* ta, uint8_t* wrapped_key,
                          uint32_t size_wrapped_key)
{
    uint32_t size_key = sizeof(ta->hmac_key);

    ta->hmac_key_initialized = 0;

    int status = fpc_ta_hw_auth_unwrap_key(wrapped_key, size_wrapped_key,
                                     ta->hmac_key, &size_key);
    if (status) {
        return status;
    }

    ta->hmac_key_initialized = 1;

    return 0;
}

int fpc_ta_hw_has_challenge()
{
    return g_hw_auth_ta.incoming_challenge != 0;
}

static int fpc_ta_hw_auth_handler(void* buffer, uint32_t size_buffer)
{
    fpc_ta_hw_auth_command_t* command = shared_cast_to(fpc_ta_hw_auth_command_t,
                                               buffer, size_buffer);

    if (!command) {
        LOGE("%s, Failed to get hw_auth_command", __func__);
        return -FPC_ERROR_INPUT;
    }

    LOGD("%s command %u", __func__, command->header.command);

    fpc_hw_auth_ta_t* ta = &g_hw_auth_ta;

    switch (command->header.command) {
    case FPC_TA_HW_AUTH_SET_AUTH_CHALLENGE:
        command->set_auth_challenge.response =
                 set_auth_challenge(ta, command->set_auth_challenge.challenge);

        break;
    case FPC_TA_HW_AUTH_GET_ENROL_CHALLENGE:
        command->get_enrol_challenge.response =
               get_enrol_challenge(ta, &command->get_enrol_challenge.challenge);
        break;
    case FPC_TA_HW_AUTH_AUTHORIZE_ENROL:
        command->authorize_enrol.response = authorize_enrol(ta,
                                            command->authorize_enrol.array,
                                            command->authorize_enrol.size);
        break;
    case FPC_TA_HW_AUTH_GET_AUTH_RESULT:
        command->get_auth_result.response = get_auth_result(ta,
                                            command->get_auth_result.array,
                                            command->get_auth_result.size);
        break;
    case FPC_TA_HW_AUTH_SET_SHARED_KEY:
        command->set_shared_key.response = set_shared_key(ta,
                                            command->set_shared_key.array,
                                            command->set_shared_key.size);
        break;
    default:
        LOGE("%s unknown command %i", __func__, command->header.command);
        return -FPC_ERROR_INPUT;
    }

    return 0;
}

static int fpc_ta_hw_auth_init(void)
{
    LOGD("%s", __func__);
    fpc_hw_auth_ta_t* hw_auth_ta = &g_hw_auth_ta;

    hw_auth_ta->bio = fpc_bio_get_handle();
    hw_auth_ta->outgoing_challenge = 0;
    hw_auth_ta->incoming_challenge = 0;
    hw_auth_ta->outgoing_timestamp = (uint64_t) -1;
    hw_auth_ta->hmac_key_initialized = 0;

    return 0;
}

fpc_ta_module_t fpc_ta_hw_auth_module = {
    .init = fpc_ta_hw_auth_init,
    .exit = NULL,
    .handle_message = fpc_ta_hw_auth_handler,
    .key = TARGET_FPC_TA_HW_AUTH,
};
