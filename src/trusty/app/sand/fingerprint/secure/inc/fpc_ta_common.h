/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TA_COMMON_INTERNAL_H
#define FPC_TA_COMMON_INTERNAL_H

#include "fpc_types.h"
#include "fpc_external.h"

typedef struct _fpc_ta_common_t {

    uint64_t image_timestamp;
    image_t *image;
    uint8_t check_for_bad_pixels;

} fpc_ta_common_t;

fpc_ta_common_t* fpc_common_get_handle(void);

#endif // FPC_TA_BIO_INTERNAL_H

