/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdio.h>
#include "fpc_log.h"
#include "fpc_debug.h"
#include "trusty_std.h"

void fpc_get_timestamp(uint64_t* time) {

    int64_t cur_time = 0;
    status_t rc = gettime(0, 0, &cur_time);
    if (rc) {
           cur_time = 0;
           LOGD("%s: Error: 0x%x", __func__, rc);
    }
    *time = (uint64_t)(cur_time / 1000);
#ifdef FPC_TA_DEBUG_TIMESTAMP
    LOGD("%s: time_stamp: %llu", __func__, *time);
#endif /* FPC_TA_DEBUG_TIMESTAMP */
}

int fpc_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

