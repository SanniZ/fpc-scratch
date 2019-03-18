/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/
#ifndef FPC_SENSOR_FORCE_H
#define FPC_SENSOR_FORCE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fpc_result.h"

/**
 * Read current force sensor value
 *
 * @param[out] value The force sensor value
 *
 * @return FPC_RESULT_OK if success
 */
int fpc_force_sensor_get_value(uint8_t* value);

#endif //FPC_SENSOR_FORCE_H
