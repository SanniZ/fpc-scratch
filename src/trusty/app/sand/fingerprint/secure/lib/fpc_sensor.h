/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_SENSOR_H_
#define FPC_SENSOR_H_
#include <stdint.h>
#include "fpc_external.h"
#include "fpc_sensor_types.h"
#include "fpc_result.h"

typedef struct _device_context_t device_context_t;

typedef struct {
    int cac_result; /**< The CAC result, see CAC_ERROR_*. */
} fpc_device_capture_details_t;

/*
 * Captures an image from the sensor, and stores it for use with enroll and
 * identify.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_SENSOR  - if the image capture failed because of the
 *                                communication with the sensor
 *         FPC_RESULT_BAD_QUALITY   - if image did not qualify as a finger or is
                                  of poor quality.
 *         FPC_RESULT_FINGER_LOST   - finger was lost during capture.
 *         FPC_RESULT_ERROR_UNKNOWN - if other error occurred
 */
int fpc_device_capture_image(
        device_context_t* context,
        image_t* image);

/*
 * Captures an image from the sensor, and stores it for use with enroll and
 * identify. Additional information of the result can optionally be retrieved.
 *
 * @param context      The context previously initialized with fpc_device_init
 * @param[out] image   The image buffer to store the captured image in.
 * @param[out] details Pointer to receive details from the capture or NULL.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_SENSOR  - if the image capture failed because of the
 *                                communication with the sensor
 *         FPC_RESULT_BAD_QUALITY   - if image did not qualify as a finger or is
                                  of poor quality.
 *         FPC_RESULT_FINGER_LOST   - finger was lost during capture.
 *         FPC_RESULT_ERROR_UNKNOWN - if other error occurred
 */
int fpc_device_capture_image_status_details(
        device_context_t* context,
        image_t* image,
        fpc_device_capture_details_t *details);

/*
 * Captures an image from the sensor but doesn't run it through any processing.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_SENSOR  - if the image capture failed because of the
 *                                communication with the sensor
 *         FPC_RESULT_ERROR_UNKNOWN - if other error occurred
 */
int fpc_device_capture_uncalibrated_image(
        device_context_t* context, image_t* image);


/**
 * Capture an image from the sensor for use during PN calibration
 * using a finger on the sensor. The finger must cover all finger
 * areas for successful capture.
 *
 * @param context
 * @param[out] image
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_FINGER_LOST - not all finger areas are covered
 *         FPC_RESULT_...
 */
int fpc_device_capture_pn_image(
    device_context_t* context,
    image_t* image);

/**
 * Enable or disable early stop on the cac
 *
 * @param context
 * @param[in/out] ctrl Input value is the mode that should be set on the cac.
 *                     As return value this is the previous value that was set on the cac.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_UNKNOWN
 */
#ifdef FPC_CONFIG_ENGINEERING
int fpc_device_early_stop_ctrl(device_context_t *context,
                               uint8_t *ctrl);
#endif

/*
 * Get size and initialize memory for this instance.
 *
 * Call this function twice, first with context as NULL to get the required size
 * of context. After this its up to the caller of this function to allocate
 * the requsted memory of size returned in the context_size parameter.
 *
 * Then call this function again with context pointing to a
 * buffer of requested size.
 *
 * @param[in] context, NULL to get required size of context.
 *                     Or buffer of requsted size.
 *
 * @param[out] required size of context allocation in bytes.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_MEMORY    - if context is null,
 *                                  this is expected on first call to function.
 *         FPC_RESULT_ERROR_PARAMETER - if context_size is null.
 */
int fpc_device_init(
        device_context_t *context,
        uint32_t *context_size);

/*
 * Release allocations and clean up this instance.
 * Caller is responsible for finally freeing context.
 */
void fpc_device_deinit(device_context_t *context);

/*
 * Check if the finger is removed from the sensor
 *
 * @param context[in] The context previously initialized with fpc_device_init
 *
 * @return FPC_RESULT_FINGER_LOST - if finger is not on the sensor
 *         FPC_RESULT_WAIT_TIME   - finger is on sensor, sleep and try later
 */
int fpc_sensor_check_finger_lost(device_context_t *context);

/*
 * Puts the sensor in sleep mode with finger detect. Used to prepare the
 * sensor to wake the phone from sleep.
 * @return FPC_RESULT_OK on success.
 *         FPC_RESULT_ERROR_UNKNOWN on failure.
 */
int fpc_sensor_wakeup_setup(void);

/*
 * Puts the sensor in sleep mode with finger lost detection. Only used with
 * fpc1022 and later, not supported on earlier version.
 * @return FPC_RESULT_OK on success.
 *         FPC_RESULT_ERROR_UNKNOWN on failure.
 */
int fpc_sensor_finger_lost_wakeup_setup(void);

/*
 * Used when phone "wake up" is triggered by the sensor. If qualification
 * passes the phone should be woken up.
 *
 * @return FPC_RESULT_FINGER_PRESENT - if the wake qualification passed
 *         FPC_RESULT_FINGER_LOST - if the qualification fails
 *         FPC_RESULT_WAIT_TIME   - finger qualification timed out.
 */
int fpc_sensor_wakeup_qualification(void);

/*
 * Get number of DFD plates/zones that are triggered.
 *
 * @param[in/out]  zones             - Number of zones that are present.
 *                                     Value is read out from the sensor.
 * @return         < 0               - Error
 *
 */
int fpc_sensor_get_dfd_zone_count(uint32_t* const zones);

/*
 * Get limit value for number of DFD zones that is needed to trigger a finger detect event.
 *
 * @param[in/out]  zones             - Number of zones that are expected
 *                                     by software configuration.
 * @return         < 0               - Error
 *
 */
int fpc_sensor_get_finger_detect_limit(uint32_t* const zones);

/*
 * Puts the sensor in low power deep sleep mode.
 * @return FPC_RESULT_OK on success.
 *         FPC_RESULT_ERROR_SENSOR on failure.
 */
int fpc_sensor_deep_sleep(void);

/*
 * Opens the spi communication before transfer, must be called before any
 * command that requires sensor communication.
 * @return FPC_RESULT_OK on success.
 *         FPC_RESULT_ERROR_SENSOR on failure.
 */
int fpc_sensor_communication_start(void);

/*
 * Close the spi communication after transfer, must be called after any
 * command that requires sensor communication.
 * @return FPC_RESULT_OK on success.
 *         FPC_RESULT_ERROR_SENSOR on failure.
 */
int fpc_sensor_communication_stop(void);

#endif /* FPC_SENSOR_H_ */
