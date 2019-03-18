/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef INCLUSION_GUARD_FPC_TA_BIO
#define INCLUSION_GUARD_FPC_TA_BIO

#include "fpc_ta_bio_interface.h"


typedef struct fpc_bio fpc_bio_t;

fpc_bio_t* fpc_bio_get_handle(void);


/**
 * implementation specific, will be called before storing templates, optionally
 * return a user identity cookie to store with the db.
 *
 * @return true if enrollment is currently allowed
 */
int fpc_enrollment_allowed(fpc_bio_t* bio, uint64_t* user);

#endif // INCLUSION_GUARD_FPC_TA_BIO
