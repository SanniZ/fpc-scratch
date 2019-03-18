/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef __FPC_RESULT_H__
#define __FPC_RESULT_H__

typedef enum {
    /*Not errors, part of normal execution*/
    FPC_RESULT_OK                        = 0,
    FPC_RESULT_WAIT_TIME                 = 1,
    FPC_RESULT_FINGER_PRESENT            = 2,
    FPC_RESULT_FINGER_LOST               = 3,
    FPC_RESULT_BAD_QUALITY               = 4,
    FPC_RESULT_FINGER_ALREADY_ENROLLED   = 5,

    /*Errors*/
    FPC_RESULT_ERROR_UNKNOWN             = -6,
    FPC_RESULT_ERROR_MEMORY              = -7,
    FPC_RESULT_ERROR_PARAMETER           = -8,
    FPC_RESULT_ERROR_TEST_FAILED         = -9,
    FPC_RESULT_ERROR_TIMEDOUT            = -10,
    FPC_RESULT_ERROR_SENSOR              = -11,

    FPC_RESULT_ERROR_SPI                 = -12,
    FPC_RESULT_ERROR_NOT_SUPPORTED       = -13,

    FPC_RESULT_ERROR_OTP                 = -14,
    FPC_RESULT_ERROR_STATE               = -15,

    FPC_RESULT_ERROR_PN                  = -16,
    FPC_RESULT_ERROR_DEAD_PIXELS         = -17,
    FPC_RESULT_ERROR_TEMPLATE_CORRUPTED  = -18,
} fpc_result_t;


#define SUCCESS( X ) ( (X) >= 0)
#define FAILED( X ) ( (X) < 0)

#endif // __FPC_RESULT_H__
