/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_ALGO_FORCE_H_
#define FPC_ALGO_FORCE_H_

#include <stdint.h>
#include "fpc_algo.h"

/*
 * Estimates finger force and wetness.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] image, the image sequence to estimate.
 *            The reference frame should be stored as the first image,
 *            and the current frame as the second image.
 * @param[out] force_score, Force estimate in range 0-100. A high number means
 *                          high amount of force.
 *
 * @return FPC_RESULT_OK - estimation was successful, otherwise algo error.
 */
int fpc_algo_force_estimate(
        algo_context_t* context,
        image_t *image,
        int32_t *force_score);

/*
 * Calculates finger stationarity score.
 *
 * @param[in] context, previously initialized by fpc_algo_load_configuration.
 * @param[in] image, the image sequence to estimate.
 *            The reference frame should be stored as the first image,
 *            and the current frame as the second image.
 *
 * @param[out] stationary_score,  Finger stationarity score in range 0-255. A value below
 *                                ~100 indicates that the finger is stationary on the
 *                                sensor. A higher value indicates that the finger is
 *                                sliding.
 * @param[out] finger_down_score, Finger down score in range 0-255. A value above ~110
 *                                indicates that the finger is still on its way down.
 * @param[out] finger_down_score2, Alternative finger down score in range 0-255. A value above
 *                                 ~100 indicates that the finger is still on its way down.
 *
 * @return FPC_RESULT_OK - estimation was successful, otherwise algo error.
 */
int fpc_algo_finger_stationary(
        algo_context_t* context,
        image_t *image,
        int32_t *stationary_score,
        int32_t *finger_down_score,
        int32_t *finger_down_score2);
#endif // FPC_ALGO_FORCE_H_
