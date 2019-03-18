/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 * Sensor pixel bias calibration.
 */

#ifndef __FPC_SENSOR_PN_H
#define __FPC_SENSOR_PN_H

#include "fpc_sensor_types.h"
#include "fpc_external.h"

void fpc_sensor_pn_get_size(uint32_t *size);

/**
 * Run PN image calibration.
 *
 * @param buffer
 * @param size
 *
 * @return FPC_RESULT_OK            if successful
 *         FPC_RESULT_FINGER_LOST   if not all finger areas are covered,
 *                              retry call after ~100ms wait
 *         FPC_RESULT_ERROR_SENSOR  failure
 */
int fpc_sensor_pn_calibrate(uint8_t        *buffer,
                                        const uint32_t  size);

/**
 * Load PN image from the provided buffer.
 *
 * @param pn_buffer
 * @param size
 * @param image
 *
 * @return FPC_RESULT_OK if successful
 */
int fpc_sensor_load_pn(const void     *pn_buffer,
                       const uint32_t  size,
                       image_t        *image);

/**
 * Extract PN image file content from 'image'.
 *
 * @param image
 * @param[out] buffer
 * @param size
 *
 * @return FPC_RESULT_OK if successful
 */
int fpc_sensor_image_to_pn_image(
    const image_t  *image,
    uint8_t        *buffer,
    const uint32_t  size);

#endif
