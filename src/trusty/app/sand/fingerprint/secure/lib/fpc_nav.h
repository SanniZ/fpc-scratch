/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef _FPC_NAV_H_
#define _FPC_NAV_H_

#include <stdint.h>

#include "fpc_sensor.h"
#include "fpc_nav_types.h"
#include "fpc_external.h"

typedef struct {
    device_context_t *context;
    image_t *image;
} fpc_nav_init_data_t;

/*
 * Set navigation configuration
 *
 * Navigation must be stopped before calling.
 *
 * @param[in] config, struct containing configuration data
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_STATE - Navigation is already running
 */
int fpc_navigation_set_config(const fpc_nav_config_t* config);

/* Get navigation configuration
 *
 * @param[out] config, struct containing configuration data
 * @return FPC_RESULT_OK
 */
int fpc_navigation_get_config(fpc_nav_config_t* config);

/*
 * Initialize navigation
 *
 * @param[in] init_data, struct containing navigation initialization data
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - Nav config does not match sensor.
 *         FPC_RESULT_ERROR_SENSOR - Sensor did not reply as expected.
 *         FPC_RESULT_ERROR_SPI - Failed to read write spi.
 *         FPC_RESULT_ERROR_UNKNOWN
 */
int fpc_navigation_init(const fpc_nav_init_data_t *init_data);

/*
 * Stop navigation
 *
 * @return FPC_RESULT_OK
 */
int fpc_navigation_exit(void);

/*
 * poll navigation data
 *
 *
 * @param[out] nav_data, struct containing navigation data
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_SENSOR - Sensor did not reply as expected.
 *         FPC_RESULT_ERROR_SPI - Failed to read write spi.
 *         FPC_RESULT_ERROR_UNKNOWN
 */
int fpc_navigation_poll_data(fpc_nav_data_t* nav_data);

#endif //_FPC_NAV_H_
