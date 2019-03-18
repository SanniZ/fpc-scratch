/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_BIO_INTERNAL_H
#define FPC_TA_BIO_INTERNAL_H

#include "fpc_db.h"
#include "fpc_types.h"

#include "fpc_algo.h"

typedef struct fpc_ta_match_result_t {
    /* timestamp of the image that was matched */
    uint64_t timestamp;
    /* the unique 32 bit identity of the template that was matched */
    uint32_t template_id;
    /* the unique 256 bit identity of the template that was matched */
    id256_t template_id256;
} fpc_ta_match_result_t;

void fpc_match_result_reset(fpc_ta_match_result_t* result);

struct fpc_bio {
    FingerprintDatabase_t* user_db;
    int32_t active_set_key;
    fpc_algo_identify_data_t ident_data;
    fpc_algo_enroll_data_t enroll_data;
    fpc_algo_template_t list[FPC_DB_FINGERPRINTS_IN_SET];
    uint32_t size_list;
    int disable_liveness;
    fpc_ta_match_result_t match_result;
    algo_context_t *algo_context;
};


#endif // FPC_TA_BIO_INTERNAL_H

