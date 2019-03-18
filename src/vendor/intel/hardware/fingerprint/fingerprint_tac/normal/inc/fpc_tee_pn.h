/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_PN_H
#define FPC_TEE_PN_H

#include "fpc_tee.h"
#include "fpc_tee_sensor.h"

/**
 * Get the needed allocation size for the encrypted PN file.
 *
 * @param sensor
 * @param[out] size
 *
 * @return FPC_PN_OK if successful
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_get_size(fpc_tee_sensor_t *sensor, uint32_t *size);

/**
 * Load the content from a PN file into the TA's global
 * fpc_image_data_t.
 *
 * @param sensor
 * @param pn_buffer   - Content as read from the PN file, always encrypted.
 * @param pn_size     - Number of bytes in 'pn_buffer'
 *
 * @return FPC_PN_OK            - Successful
 *        -FPC_PN_FAILED        - Any other error such as failed sensor
 *                                communication or CAC failure.
 *        -FPC_ERROR_ALLOC
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_load(fpc_tee_sensor_t* sensor,
                    const void *pn_buffer, uint32_t pn_size);

/**
 * Run PN calibration having a finger on the sensor by repeatedly
 * making calls to this function until FPC_PN_OK is returned. The PN
 * file content is then available by calling
 * fpc_tee_pn_calibrate_finger_end().
 *
 * NOTE:
 * Regardless of the outcome of this function,
 * fpc_tee_pn_calibrate_finger_end() must be called to release
 * resources.
 *
 * @param sensor
 * @param[out] image_decision   - 0 - injected image rejected
 *                                1 - injected image accepted
 * @param[out] image_quality    - Quality of image being injected [0:100]
 * @param[out] pn_quality       - Quality of PN image being created, [0:100]
 * @param[out] progress         - Overall PN calibration progress, [0:100]
 *
 * @return FPC_PN_OK                - Calibration has ended successfully.
 *        -FPC_PN_RETRY_CALIBRATION - Calibration is in progress but a new
 *                                    call to his function must be made.
 *        -FPC_PN_FAILED            - Any other error such as failed sensor
 *                                    communication or CAC failure.
 *        -FPC_ERROR_ALLOC
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_calibrate_finger(fpc_tee_sensor_t *sensor,
                                int32_t *image_decision,
                                int32_t *image_quality,
                                int32_t *pn_quality,
                                int32_t *progress);

/**
 * Retrieve the PN file content after successful PN calibration using
 * fpc_tee_pn_calibrate_finger() or cleanup resources in case of
 * previous failure.
 *
 * NOTE:
 * To clean up in BioLib, this function must always be called after
 * fpc_tee_pn_calibrate_finger() has been called i.e. also when that
 * function has failed. In that case 'pn_buffer' and 'pn_size' should
 * be set to NULL and 0 respectively.
 *
 * @param sensor
 * @param[out] pn_buffer   - Encrypted PN file content.
 * @param pn_size          - Size in bytes of 'pn_buffer'.
 *                           Value from fpc_tee_pn_get_size().
 *
 * @return FPC_PN_OK       - Valid PN file content is in 'pn_buffer'
 *        -FPC_PN_MEMORY   - Failed to allocate memory
 *        -FPC_PN_FAILED   - Any other error.
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_calibrate_finger_end(fpc_tee_sensor_t *sensor,
                                    void             *pn_buffer,
                                    const uint32_t    pn_size);

/**
 * Get challenge used for PN calibration using a finger on the sensor.
 *
 * @param sensor
 * @param[out] challenge  - The new challenge
 *
 * @return FPC_PN_OK
 *        -FPC_ERROR_IO
 *        -FPC_ERROR_NOT_INITIALIZED
 */
int fpc_tee_pn_get_challenge(fpc_tee_sensor_t *sensor, uint64_t *challenge);

/**
 * Authorize the given token when doing PN calibration using a finger.
 *
 * @param sensor
 * @param token
 * @param size_token
 *
 * @return FPC_PN_OK
 *        -FPC_ERROR_ALLOC
 *        -FPC_ERROR_INPUT
 *        -FPC_ERROR_TIMEDOUT
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_authorize(fpc_tee_sensor_t *sensor, const uint8_t* token,
        uint32_t size_token);

/**
 * Run PN calibration with a rubber stamp on the sensor.
 *
 * @param sensor
 * @param image       - Encrypted PN file content.
 * @param image_size  - Size in bytes of 'image'.
 *
 * @return FPC_PN_OK
 *        -FPC_PN_MEMORY            - Failed to allocate memory
 *        -FPC_PN_FAILED            - Any other error
 *        -FPC_ERROR_RESET_HARDWARE - Sensor communication error
 *         Command routing errors:
 *           -FPC_ERROR_NOT_INITIALIZED
 *           -FPC_ERROR_INPUT
 *           -FPC_ERROR_CONFIG
 */
int fpc_tee_pn_calibrate(fpc_tee_sensor_t *sensor,
                         void *image, const uint32_t image_size);

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_pn_get_unencrypted_size(fpc_tee_sensor_t *sensor, uint32_t *size);
int fpc_tee_pn_get_unencrypted_image(fpc_tee_sensor_t *sensor,
                                     void             *image,
                                     const uint32_t    size);
#endif

#endif
