/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef _FPC_TEST_H_
#define _FPC_TEST_H_

#include "stdint.h"
#include "fpc_external.h"

typedef enum {
    AFD_CALIBRATION_TEST,
    AFD_CALIBRATION_RUBBER_STAMP_TEST,
    AFD_RUBBER_STAMP_TEST,
} fpc_test_afd_test_type_t;

/*
 * Run inititialization of self test. The init should set the sensor in a known state eg. it should
 * clear any active irqs.
 *
 * @param[out] result, Not really used kept for future use. Set to IRQ register content on success
 *                     otherwise failure
 *
 * @return TEST_OK or error
 */
int fpc_test_self_test_init(uint32_t* result);

/*
 * Run self test.
 *
 * @param[out] result, result from the self test
 *               FPC_SELFTEST_FAIL
 *               FPC_SELFTEST_SUCCESS
 *
 * @return TEST_OK or error
 */
int fpc_test_run_self_test(uint32_t* result);

/*
 * Run cleanup of self test (clear irq).
 *
 * @param[out] result, Not really used kept for future use. Set to IRQ register content on success
 *                     otherwise failure
 *
 * @return TEST_OK or error
 */
int fpc_test_self_test_cleanup(uint32_t* result);

/*
 * Run checkerboard test.
 *
 * @param[out] image, image captured by this test.
 *
 * @param[out] result, bitmapped result from the checkerboard test
 *                  0x01    FPC_TYPE1_MEDIAN_ERROR
 *                  0x02    FPC_TYPE2_MEDIAN_ERROR
 *                  0x04    Nbr dead pixels
 *                  0x08    Nbr dead pixels in finger detect areas error
 *
 * @return TEST_OK
*/
int fpc_test_run_checkerboard_test(image_t* image, uint32_t* result);

/*
 * Is image quality test supported.
 *
 * @return 1  - Yes
 *         0  - No
*/
int fpc_test_is_image_quality_test_supported(void);

/*
 * Run image quality test.
 *
 * @param[out] image, image captured by this test.
 *
 * @param[out] result <TBD>
 *
 * @return TEST_OK
*/
int fpc_test_run_image_quality_test(image_t* image, uint32_t* result);

/*
 * Is reset pixel test supported.
 *
 * @return 1  - Yes
 *         0  - No
*/
int fpc_test_is_reset_pixel_test_supported(void);

/*
 * Run reset pixel test.
 *
 * @param[out] result <TBD>
 *
 * @return TEST_OK
*/
int fpc_test_run_reset_pixel_test(uint32_t* result);

/*
 * Run dead pixel update.
 *
 * @param[out] image, image captured by this test.
 *
 * @return FPC_RESULT_OK
 */

int fpc_test_dead_pixel_update(image_t *image);

/*
 * Is AFD test supported.
 *
 * @param[in]  test_type   AFD test type
 *
 * @return 1  - Yes
 *         0  - No
*/
int fpc_test_is_afd_test_supported(fpc_test_afd_test_type_t test_type);

/*
 * Run AFD test.
 *
 * @param[in]  test_type   AFD test type
 * @param[out] result      result from the test
 *                         0 means success, fail otherwise
 *
 * @return TEST_OK
*/
int fpc_test_run_afd_test(fpc_test_afd_test_type_t test_type, uint32_t* result);

/*
 * Is module quality test supported.
 *
 * @return 1  - Yes
 *         0  - No
*/
int fpc_test_is_module_quality_test_supported(void);

/*
 * Run module quality test.
 *
 * @param[in/out] image             image to use for zebra image quality test
 * @param[in] snr_limit_preset      snr limit preset
 * @param[in] snr_cropping_left     snr cropping left
 * @param[in] snr_cropping_top      snr cropping top
 * @param[in] snr_cropping_right    snr cropping right
 * @param[in] snr_cropping_bottom   snr cropping bottom
 * @param[out] result               result from the test
 *                                  0 means success, fail otherwise
 * @param[out] snr                  resulting snr value
 * @param[out] snr_error            result from the snr_db
 *
 * @return TEST_OK
*/
int fpc_test_run_module_quality_test(image_t* image,
                                     uint32_t snr_limit_preset,
                                     uint32_t snr_cropping_left,
                                     uint32_t snr_cropping_top,
                                     uint32_t snr_cropping_right,
                                     uint32_t snr_cropping_bottom,
                                     uint32_t* result,
                                     uint32_t* snr,
                                     uint32_t* snr_error);

/*
 * Is PN (Pixel Noise) image test supported.
 *
 * @return 1  - Yes
 *         0  - No
*/
int fpc_test_is_pn_image_test_supported(void);

/*
 * Run PN (Pixel Noise) image test.
 *
 * @param[in] image        Image to use for PN image test.
 * @param[out] result      Bitfield result from the test.
 *                         A high bit (1) indicates failure, a low bit (0) inicates success.
 *                         Bit 0 high (0x01) : PN Image is empty.
 *                         Bit 1 high (0x02) : PN Metadata is empty.
 *
 * @return TEST_OK
*/
int fpc_test_run_pn_image_test(const image_t* image, uint32_t* result);

#endif /* _FPC_TEST_H_ */
