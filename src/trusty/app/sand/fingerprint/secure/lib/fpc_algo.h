/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_ALGO_H_
#define FPC_ALGO_H_

#include <stdint.h>
#include "fpc_external.h"
#include "fpc_result.h"

#define FPC_ALGO_MAX_TEMPLATE_HANDLES 5

typedef enum {
    FPC_ALGO_IDENTIFY_NO_MATCH,
    FPC_ALGO_IDENTIFY_MATCH,
} fpc_algo_identify_result_t;

typedef enum {
    FPC_ALGO_NO_UPDATED_TEMPLATE,
    FPC_ALGO_UPDATED_TEMPLATE,
} fpc_algo_template_update_result_t;

typedef enum {
    FPC_ALGO_ENROLL_SUCCESS,
    FPC_ALGO_ENROLL_IMAGE_TOO_SIMILAR,
    FPC_ALGO_ENROLL_TOO_MANY_ATTEMPTS,
    FPC_ALGO_ENROLL_TOO_MANY_FAILED_ATTEMPTS,
    FPC_ALGO_ENROLL_FAIL_LOW_QUALITY,
    FPC_ALGO_ENROLL_FAIL_LOW_COVERAGE,
    FPC_ALGO_ENROLL_FAIL_LOW_QUALITY_AND_LOW_COVERAGE,
    FPC_ALGO_ENROLL_FAIL_ALREADY_ENROLLED,
} fpc_algo_enroll_result_t;

typedef struct {
   /* Progress of the current enroll process in percent */
    uint32_t progress;
    /* Quality for the image*/
    uint32_t quality;
    /* Status of current enroll attempt */
    fpc_algo_enroll_result_t result;
    /* Size of the enrolled template */
    uint32_t enrolled_template_size;
     /* Number of successful enroll attempts so far */
    uint32_t coverage;
    /* Used to indicate that touches are too similar */
    int8_t user_touches_too_immobile;
    /* Number of remaining touches */
    uint32_t remaining_touches;
} fpc_algo_enroll_data_t;

/*
 * Structure for templates stored in the database
 */
typedef struct {
    /* Pointer to a buffer for template data */
    uint8_t* tpl;
    /* Size of the buffer for template data */
    uint32_t size;
} fpc_algo_template_t;

/* Data from the identification attempt */
typedef struct {
    /* Result of the identification attempt */
    fpc_algo_identify_result_t result;
    /* Matching score */
    uint32_t score;
    /* Index of the identification template */
    uint32_t index;
    /* Size of the update template if one exits */
    uint32_t updated_template_size;
    /* Estimated sensor coverage and image quality of the indentify image */
    int32_t quality;
    int32_t coverage;
    /* Number of zones covered on the sensor */
    int32_t covered_zones;
    /* List of template indices that were updated */
    uint32_t updated_template_indices[FPC_ALGO_MAX_TEMPLATE_HANDLES];
    /* List of sizes for the templates that were updated */
    uint32_t updated_template_sizes[FPC_ALGO_MAX_TEMPLATE_HANDLES];
    /* Number of templates that were updated */
    uint32_t num_updated_templates;
    /* Result of the template update */
    fpc_algo_template_update_result_t update_result;
} fpc_algo_identify_data_t;

/* Struct for PN image calibration using finger */
typedef struct {
    int32_t image_decision;     /* 0 - injected image rejected,
                                   1 - injected image accepted */
    int32_t image_quality;      /* Quality of image being injected [0:100] */
    int32_t pn_quality;         /* Quality of PN image being created, [0:100] */
    int32_t progress;           /* Overall PN calibration progress, [0:100] */
} fpc_algo_pn_add_image_data_t;

typedef struct _algo_context_t algo_context_t;

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
 *         FPC_RESULT_ERROR_MEMORY - if context is null,
 *                                 this is expected on first call to function.
 *         FPC_RESULT_PARAMETER    - if context_size is null.
 */

int fpc_algo_load_configuration(
    algo_context_t *context,
    uint32_t *context_size);

/*
 * Release allocations and clean up this instance.
 * Caller is responsible for finally freeing context.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 */
void fpc_algo_cleanup(algo_context_t *context);

/*
 * Begin the enrollment of a new finger.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 *
 * @return FPC_RESULT_OK
 *
 */
int fpc_algo_begin_enroll(algo_context_t *algo);

/*
 * Add the image to the current enrollment process.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] image, the image to add to the enrollment.
 * @param[out] enroll_data, the data from the enrollment.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - if data is NULL
 *         FPC_RESULT_ERROR_MEMORY - if memory allocation failed during the enrollment
 */

int fpc_algo_enroll(
        algo_context_t *context,
        image_t* img,
        fpc_algo_enroll_data_t *enroll_data);
/*
 * Ends the enrollment and returns the enrolled finger template.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] img, image aquired from capture.
 * @param[out] tpl, a buffer with the template data for the enrolled finger.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - if template is NULL or the size of the
 *                                    buffer is too small
 *         FPC_RESULT_ERROR_MEMORY    - if the template extraction failed
 */
int fpc_algo_end_enroll(
        algo_context_t* context,
        fpc_algo_template_t* tpl);

/*
 * Begin identify for the given template candidates.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] candidates, the templates of the candidates to verify against
 * @param[in] num_tpl, number of templates in the array.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - if candidates is a NULL pointer, size is
 *         zero or security is an invalid number
 *         FPC_RESULT_ERROR_MEMORY - if memory allocation fails during
 *                                 the identification process
*/
int fpc_algo_begin_identify(
        algo_context_t* context,
        fpc_algo_template_t* candidates,
        uint32_t num_tpl);

/*
 * Use the currently stored image for identification.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] img, image aquired from capture.
 * @param[out] Identification data, decision, sensor coverage etc.
 * @param[in] 0 to disable liveness. 1 to use.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - if data is a null pointer
 *         FPC_RESULT_ERROR_MEMORY - if the memory allocation failed during the
 *                                 identification
*/
int fpc_algo_identify(
    algo_context_t *context,
    image_t* img,
    fpc_algo_identify_data_t* data,
    uint8_t liveness);

/*
 * Check if a template update occured.
 *
 * @param[in] context, the current algo context
 * @param[in/out] data, container of the identify result
 *
 * @return FPC_ALGO_OK
 *         FPC_ALGO_ERROR_GENERAL - If there is an error
 */
int fpc_algo_identify_update(algo_context_t *context,
        fpc_algo_identify_data_t* data);

/*
 * End the started identification. If there was a template update during the
 * identification process, the given template struct will be filled with
 * template data. If a null pointer is given the update will be discarded.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 *
 * @return FPC_RESULT_OK
 *         FPC_RESULT_ERROR_PARAMETER - if template is NULL or the size of the
 *         buffer too small, (the needed size is returned in the template data)
 */
int fpc_algo_end_identify(
        algo_context_t* context);

/*
 * Called on the image to give a enhanced image, the image is returned in the
 * provided buffer.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] image, the capture raw image
 * @param[out] buffer, buffer for the enhanced image
 * @param[in] size, size of the buffer
 *
 * @return FPC_RESULT_OK or,
 *         FPC_RESULT_ERROR_PARAMETER if the image or buffer is null or the size
 *         is too small,
 *         otherwise algo error.
 */
int  fpc_algo_storage_retrieve_enhanced_image(
        algo_context_t* context,
        image_t* image,
        uint8_t* buffer,
        uint32_t size);

/*
 * Qualifies the given image for use with identify and enroll. If the quality
 * of the image is to low this function will return FPC_ALGO_ERROR_BAD_QUALITY.
 *
 * @param[in] image, the image to qualify
 *
 * @return FPC_RESULT_OK - if the image has high enough quality.
 *         FPC_RESULT_BAD_QUALITY - if the image has too low quality
 */
int fpc_algo_qualify_image(image_t* image);

/*
 * Update the given list of template structs will be filled with
 * template data. If a null pointer is given the update will be discarded.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in/out] templates, a list of buffers for the updated templates
 * @param[in] template_indices, a list of template indices to update template for
 * @param[in] num_tpls, the number of templates to update
 *
 * @return FPC_RESULT_OK
 *         FPC_ERROR_PARAMETER - if templates is NULL or the size of the buffer
 *         is too small, (the needed size is returned in the template data)
 */
int fpc_algo_update_templates(
        algo_context_t *context,
        fpc_algo_template_t** templates,
        uint32_t *template_indices,
        uint32_t num_tpls);

/**
 * Run PN calibration having a finger on the sensor by repeatedly
 * making calls to this function adding one image at a time. When
 * pn_add_image_data->progress reaches 100% the PN image is available
 * by calling fpc_algo_pn_calibrate_finger_end().
 * NOTE:
 * Regardless of the outcome of this function, fpc_algo_pn_calibrate_finger_end()
 * must always be called to release resources.
 *
 * @param[in]  context - Previously initialized by fpc_algo_load_configuration.
 * @param[in]  image   - Image to add to the PN calibration.
 * @param[out] pn_add_image_data - The result from adding 'image'.
 *
 * @return FPC_RESULT_OK - A new call to this function must be made until
 *                         pn_add_image_data->progress reaches 100%.
 *         FPC_RESULT_ERROR_PARAMETER - If any parameter is NULL.
 *         FPC_RESULT_ERROR_...       - Any other error.
 */
int fpc_algo_pn_calibrate_finger(algo_context_t *context,
                                 image_t *image,
                                 fpc_algo_pn_add_image_data_t *pn_add_image_data);

/**
 * End PN calibration started in fpc_algo_pn_calibrate_finger().
 *
 * @param[in]  context - Previously initialized by fpc_algo_load_configuration.
 * @param[in]  image   - Image where the new PN image will be inserted.
 *
 * @return FPC_RESULT_OK - PN image successfully updated in 'image'
 *         FPC_RESULT_ERROR_PARAMETER - If any parameter is NULL.
 *         FPC_RESULT_ERROR_...       - Any other error.
 */
int fpc_algo_pn_calibrate_finger_end(algo_context_t *context,
                                     image_t *image);

#endif /* FPC_ALGO_H_ */
