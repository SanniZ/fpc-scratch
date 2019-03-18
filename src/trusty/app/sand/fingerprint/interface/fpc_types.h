/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TYPES
#define INCLUSION_GUARD_FPC_TYPES

enum FPC_ERROR {
    FPC_ERROR_INPUT = 1,
    FPC_ERROR_TIMEDOUT = 2,
    FPC_ERROR_ALLOC = 3,
    FPC_ERROR_COMM = 4,
    FPC_ERROR_NOSPACE = 5,
    FPC_ERROR_IO = 6,
    FPC_ERROR_CANCELLED = 7,
    FPC_ERROR_NOENTITY = 8,
    FPC_ERROR_HARDWARE = 9,
    FPC_ERROR_CONFIG = 10,
    FPC_ERROR_NOT_INITIALIZED = 11,
    FPC_ERROR_RESET_HARDWARE = 12,
    FPC_ERROR_PN = 13,
    FPC_ERROR_SENSOR_BROKEN = 14,
    FPC_ERROR_TOO_MANY_DEAD_PIXELS = 15,
    FPC_ERROR_CRYPTO = 16,
    FPC_ERROR_DB = 17,
    FPC_ERROR_TEMPLATE_CORRUPTED = 18,
};

enum {
    FPC_CAPTURE_OK = 0,
    FPC_CAPTURE_FINGER_LOST = 1,
    FPC_CAPTURE_BAD_QUALITY = 2,
    FPC_CAPTURE_QUALIFY_ABORT = 3,
    FPC_CAPTURE_FINGER_STUCK = 4,
    FPC_CAPTURE_RETRY = 5,
};

enum {
    FPC_ENROL_COMPLETED = 0,
    FPC_ENROL_PROGRESS = 1,
    FPC_ENROL_FAILED_COULD_NOT_COMPLETE = 2,
    FPC_ENROL_FAILED_ALREADY_ENROLED = 3,
    FPC_ENROL_IMAGE_LOW_COVERAGE = 4,
    FPC_ENROL_IMAGE_TOO_SIMILAR = 5,
    FPC_ENROL_IMAGE_LOW_QUALITY = 6,
};

enum {
    FPC_PN_OK = 0,
    FPC_PN_FAILED = 1001,
    FPC_PN_RETRY_CALIBRATION = 1002,
    FPC_PN_MEMORY = 1003,
};

#endif // INCLUSION_GUARD_FPC_TYPES
