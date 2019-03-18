/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_SENSOR_H
#define FPC_TEE_SENSOR_H

#include "fpc_tee.h"
#include "fpc_hw_identification_types.h"

typedef struct fpc_tee_sensor fpc_tee_sensor_t;

fpc_tee_sensor_t* fpc_tee_sensor_init(fpc_tee_t* tee);
void fpc_tee_sensor_release(fpc_tee_sensor_t* sensor);

/*
* Request TA side to capture and image from the sensor.

* @param image_qualifier, Indicates if image quality checks should be performed for captured image.
*	1 = enabled
*	0 = disabled
*
* @return ...
*/
int fpc_tee_capture_image(fpc_tee_sensor_t* sensor);

/**
 * Get last CAC result code.
 *
 * @param sensor The sensor struct as returned by fpc_tee_sensor_init
 *
 * @return CAC_SUCCESS
 *         CAC_ERROR_*
 */
int32_t fpc_tee_get_last_cac_result(fpc_tee_sensor_t *sensor);

/**
 * Wait for finger to be removed from the sensor
 *
 * This function have two different implementations depending on hardware and configuration.
 *
 * If FPC_CONFIG_FINGER_LOST_INTERRUPT is supported and enabled the function will wait
 * forever or until cancelled until finger is removed from the sensor. It will do so by
 * setting up an interrupt and wait for it.
 *
 * If FPC_CONFIG_FINGER_LOST_INTERRUPT is not enabled it will start a poll loop which will
 * be aborted after some time (typically ~2s) if the finger is not removed.
 *
 * @return FPC_CAPTURE_FINGER_STUCK if poll loop was aborted
 *         0                        if finger was removed
 *         -FPC_ERROR_CANCELLED     if the check was cancelled
 *
 */
int fpc_tee_wait_finger_lost(fpc_tee_sensor_t* sensor);
int fpc_tee_wait_finger_down(fpc_tee_sensor_t* sensor);
int fpc_tee_capture_snapshot(fpc_tee_sensor_t* sensor);
int fpc_tee_set_cancel(fpc_tee_sensor_t* sensor);
int fpc_tee_clear_cancel(fpc_tee_sensor_t* sensor);
int fpc_tee_wait_irq(fpc_tee_sensor_t* sensor, int irq_value);
int fpc_tee_sensor_cancelled(fpc_tee_sensor_t* sensor);
int fpc_tee_status_irq(fpc_tee_sensor_t* sensor);
int fpc_tee_is_otp_supported(fpc_tee_sensor_t* sensor, int* result);
int fpc_tee_get_sensor_otp_info(fpc_tee_sensor_t* sensor, fpc_hw_module_info_t* otp_info);
#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_early_stop_ctrl(fpc_tee_sensor_t *sensor, uint8_t *ctrl);
#endif
#if FPC_CONFIG_FORCE_SENSOR == 1

int fpc_tee_wait_for_button_down_force(fpc_tee_sensor_t* sensor,
                                       uint32_t force_button_down_timeout_ms,
                                       uint8_t force_button_down_threshold);
int fpc_tee_wait_for_button_up_force(fpc_tee_sensor_t* sensor,
                                     uint32_t force_button_up_timeout_ms,
                                     uint8_t force_button_up_threshold);
#endif

/**
 * Reads the force sensor value from ADC
 *
 * @param sensor
 * @param[out] value - value between 0-255 representing the pressure force
 *
 * @return 0 if successful, if not the error code
 */
int fpc_tee_get_sensor_force_value(fpc_tee_sensor_t* sensor, uint8_t* value);

/**
 * Checks if force sensor is supported
 *
 * @param sensor
 * @param[out] is_supported - 1 if force sensor is supported, 0 otherwise
 *
 * @return 0 if successful, if not the error code
 */
int fpc_tee_is_sensor_force_supported(fpc_tee_sensor_t* sensor, uint8_t* is_supported);
#endif // FPC_TEE_SENSOR_H
