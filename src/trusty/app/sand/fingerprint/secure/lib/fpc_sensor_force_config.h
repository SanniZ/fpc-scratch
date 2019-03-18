/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_SENSOR_FORCE_CONFIGH
#define FPC_SENSOR_FORCE_CONFIGH

#include <stdint.h>
#include <stdbool.h>

/**
 * Probe if force sensor is available
 *
 * @return true if force sensor if available, false otherwise
 */
static inline bool fpc_force_sensor_available(void)
{
#if (FPC_CONFIG_FORCE_SENSOR == 1)
    return true;
#else
    return false;
#endif
}

#endif //FPC_SENSOR_FORCE_CONFIGH
