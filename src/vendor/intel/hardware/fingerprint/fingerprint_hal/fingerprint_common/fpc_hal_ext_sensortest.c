/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "fpc_types.h"
#include "fpc_hal_ext_sensortest.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"
#include "container_of.h"

#include "fpc_ta_sensortest_interface.h"
#include "fpc_tee_sensortest.h"
#include "fpc_hal_ext_engineering.h"
#include "fpc_hal_private.h"

#define STRLEN(s) (s == NULL ? 0 : strlen(s))
#define STRNCMP(s1, s2) strncmp(s1, s2, strlen(s2))
//#define ENABLE_AFD_CALIBRATION_RUBBER_STAMP_TEST
//#define ENABLE_AFD_RUBBER_STAMP_TEST
#define STABILIZATION_MS 500

// Test cases
static const char* SELF_TEST_TITLE = "Self Test";
static const char* CHECKERBOARD_TEST_TITLE = "Checkerboard Test";
static const char* IMAGE_QUALITY_TEST_TITLE = "Image Quality Test";
static const char* IMAGE_RESET_PIXEL_TEST_TITLE = "Image Reset Pixel Test";
static const char* AFD_CALIBRATION_TEST_TITLE = "AFD Calibration Test";
static const char* AFD_CALIBRATION_RUBBER_STAMP_TEST_TITLE = "AFD Calibration Rubber Stamp Test";
static const char* AFD_RUBBER_STAMP_TEST_TITLE = "AFD Rubber Stamp Test";
static const char* MODULE_QUALITY_TEST_TITLE = "Module Quality Test";
static const char* OTP_VALIDATION_TEST_TITLE = "OTP Validation Test";
static const char* PN_IMAGE_TEST_TITLE = "PN Image Test";

// Test description
static const char* SELF_TEST_DES = "Tests the sensor response and interrupt signal";
static const char* CHECKERBOARD_TEST_DES = "Takes an sensor internal image and analyze to find dead pixels";
static const char* IMAGE_QUALITY_TEST_DES = "Takes an image with square pattern stamp and analyze to find coating issues";
static const char* IMAGE_RESET_PIXEL_TEST_DES = "Takes an sensor internal image and analyze to find faulty pixels";
static const char* AFD_CALIBRATION_TEST_DES = "Tests the calibration of AFD";
#ifdef ENABLE_AFD_CALIBRATION_RUBBER_STAMP_TEST
static const char* AFD_CALIBRATION_RUBBER_STAMP_TEST_DES = "Tests the calibration of AFD with rubber stamp";
#endif
#ifdef ENABLE_AFD_RUBBER_STAMP_TEST
static const char* AFD_RUBBER_STAMP_TEST_DES = "Tests the AFD on module level";
#endif
static const char* MODULE_QUALITY_TEST_DES = "Takes an image with zebra pattern stamp and determine the SNR (signal to noise ratio) value";
static const char* OTP_VALIDATION_TEST_DES = "Checks the OTP memory for manufacturing defects and the wafer contains relevant information";
static const char* PN_IMAGE_TEST_DES = "Checks the valid PN image stored";

// Stamp type

static const char* IMAGE_QUALITY_TEST_STAMP = "Square pattern stamp";
#ifdef ENABLE_AFD_CALIBRATION_RUBBER_STAMP_TEST
static const char* AFD_CALIBRATION_RUBBER_STAMP_TEST_STAMP = "Stamp covers 4 AFD zones";
#endif
#ifdef ENABLE_AFD_RUBBER_STAMP_TEST
static const char* AFD_RUBBER_STAMP_TEST_STAMP = "Stamp covers 4 AFD zones";
#endif
static const char* MODULE_QUALITY_TEST_STAMP = "Zebra pattern stamp";

/* Needs to be in sync with fpc_sensor_info.h */
#define FPC_PRODUCT_TYPE1035  6
#define FPC_PRODUCT_TYPE1245 10
#define FPC_PRODUCT_TYPE1265 14
#define FPC_PRODUCT_TYPE1268 15
#define FPC_PRODUCT_TYPE1028 16
#define FPC_PRODUCT_TYPE1075 17
#define FPC_PRODUCT_TYPE1263 20
#define FPC_PRODUCT_TYPE1262 21
#define FPC_PRODUCT_TYPE1266 22
#define FPC_PRODUCT_TYPE1264 23
#define FPC_PRODUCT_TYPE1272 24
#define FPC_PRODUCT_TYPE1228 25
#define FPC_PRODUCT_TYPE1267 26
#define FPC_PRODUCT_TYPE1228_G175 36
#define FPC_PRODUCT_TYPE1272_G175 38

const struct module_quality_test_limit {
    int32_t product_type;
    float snr_threshold;
    uint32_t snr_limit_preset;
    uint32_t snr_cropping_left;
    uint32_t snr_cropping_top;
    uint32_t snr_cropping_right;
    uint32_t snr_cropping_bottom;
} module_quality_test_limits[] = {
    {FPC_PRODUCT_TYPE1028,       9.0, 0, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1035,      11.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1075,       9.0, 0, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1228,       7.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1228_G175,  7.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1245,       3.0, 0, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1262,       7.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1263,       7.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1264,       7.0, 1, 4, 4, 4, 4},
    {FPC_PRODUCT_TYPE1265,       7.0, 1, 4, 4, 4, 4},
    {FPC_PRODUCT_TYPE1266,       7.0, 1, 4, 4, 4, 4},
    {FPC_PRODUCT_TYPE1267,       7.0, 1, 4, 4, 4, 4},
    {FPC_PRODUCT_TYPE1268,       7.0, 2, 4, 4, 4, 4},
    {FPC_PRODUCT_TYPE1272,       7.0, 1, 0, 0, 0, 0},
    {FPC_PRODUCT_TYPE1272_G175,  7.0, 1, 0, 0, 0, 0},
    {   -1,   0, 0, 0, 0, 0, 0}, // End of the list
};

typedef struct {
    fpc_hal_ext_sensortest_t sensortest;

    fpc_hal_ext_sensortest_test_t test;
    fpc_hal_ext_sensortest_test_input_t test_input;
    void* test_cb_ctx;
    fpc_test_result_cb_t test_result_cb;

    bool wait_for_finger;
    bool uncalibrated;
    void* capture_cb_ctx;
    fpc_capture_acquired_cb_t capture_acquired_cb;
    fpc_capture_error_cb_t capture_error_cb;

    fpc_hal_common_t* hal;
    fpc_engineering_t* engineering;
} sensortest_module_t;

static int get_sensor_info(fpc_hal_ext_sensortest_t* self, fpc_hw_module_info_t* info)
{
    LOGD("%s", __func__);
    sensortest_module_t* module = (sensortest_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    int status = fpc_tee_get_sensor_otp_info(module->hal->sensor, info);
    if (status) {
        LOGE("%s, Failed to fetch hw otp data code: %d\n", __func__, status);
    }

    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
    return status;
}

static int add_sensor_test(fpc_hal_ext_sensortest_tests_t* sensor_tests,
                                  const char* name,
                                  const char* description,
                                  bool wait_for_finger_down,
                                  const char* rubber_stamp_type)
{
    LOGD("%s", __func__);
    int status = 0;
    int size = sensor_tests->size;
    if(size < FPC_HAL_EXT_SENSORTEST_TESTS_MAX) {
        if (name != NULL) {
            strlcpy(sensor_tests->tests[size].name, name, FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX);
        }

        if (description != NULL) {
            strlcpy(sensor_tests->tests[size].description, description,
                    FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX);
        }

        sensor_tests->tests[size].wait_for_finger_down = wait_for_finger_down;

        if (rubber_stamp_type != NULL) {
            strlcpy(sensor_tests->tests[size].rubber_stamp_type, rubber_stamp_type,
                    FPC_HAL_EXT_SENSORTEST_TEST_STAMP_TYPE_MAX);
        }

        sensor_tests->size = size + 1;
    } else {
        LOGE("%s, size >= TESTS_MAX\n", __func__);
        status = -FPC_ERROR_ALLOC;
    }
    return status;
}

static int get_sensor_tests(fpc_hal_ext_sensortest_t* self,
                                   fpc_hal_ext_sensortest_tests_t* sensor_tests)
{
    LOGD("%s", __func__);
    int status = 0;
    int32_t is_supported = 0;
    sensortest_module_t* module = (sensortest_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    if (add_sensor_test(sensor_tests, SELF_TEST_TITLE, SELF_TEST_DES, false, 0) != 0) {
        goto out;
    }

    if (add_sensor_test(sensor_tests,
                        CHECKERBOARD_TEST_TITLE, CHECKERBOARD_TEST_DES, false, NULL) != 0) {
        goto out;
    }

    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests,
                            IMAGE_QUALITY_TEST_TITLE,
                            IMAGE_QUALITY_TEST_DES, true, IMAGE_QUALITY_TEST_STAMP) != 0) {
            goto out;
        }
    }

    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_RESET_PIXEL_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests,
                            IMAGE_RESET_PIXEL_TEST_TITLE,
                            IMAGE_RESET_PIXEL_TEST_DES, false, NULL) != 0) {
            goto out;
        }
    }

    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests,
                            AFD_CALIBRATION_TEST_TITLE, AFD_CALIBRATION_TEST_DES, false, NULL) != 0) {
            goto out;
        }
    }

#ifdef ENABLE_AFD_CALIBRATION_RUBBER_STAMP_TEST
    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests,
                            AFD_CALIBRATION_RUBBER_STAMP_TEST_TITLE,
                            AFD_CALIBRATION_RUBBER_STAMP_TEST_DES,
                            true, AFD_CALIBRATION_RUBBER_STAMP_TEST_STAMP) != 0) {
            goto out;
        }
    }
#endif

#ifdef ENABLE_AFD_RUBBER_STAMP_TEST
    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests, AFD_RUBBER_STAMP_TEST_TITLE,
                            AFD_RUBBER_STAMP_TEST_DES, true, AFD_RUBBER_STAMP_TEST_STAMP) != 0) {
            goto out;
        }
    }
#endif

    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_MODULE_QUALITY_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests, MODULE_QUALITY_TEST_TITLE,
                            MODULE_QUALITY_TEST_DES, true, MODULE_QUALITY_TEST_STAMP) != 0) {
            goto out;
        }
    }

    status = fpc_tee_is_otp_supported(module->hal->sensor, &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests, OTP_VALIDATION_TEST_TITLE,
                            OTP_VALIDATION_TEST_DES, false, NULL) != 0) {
            goto out;
        }
    }

    status = fpc_tee_sensortest_is_test_supported(module->hal->sensor,
                                                  FPC_TEE_SENSORTEST_PN_IMAGE_TEST,
                                                  &is_supported);
    if (!status && is_supported == 1) {
        if (add_sensor_test(sensor_tests, PN_IMAGE_TEST_TITLE,
                            PN_IMAGE_TEST_DES, true, NULL) != 0) {
            goto out;
        }
    }

out:
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
    return status;
}

static int get_module_quality_test_limit(sensortest_module_t* module,
                                         float* snr_threshold,
                                         uint32_t* snr_limit_preset,
                                         uint32_t* snr_cropping_left,
                                         uint32_t* snr_cropping_top,
                                         uint32_t* snr_cropping_right,
                                         uint32_t* snr_cropping_bottom,
                                         uint32_t* stablization_ms)
{
    fpc_hw_module_info_t info;
    int status = fpc_tee_get_sensor_otp_info(module->hal->sensor, &info);
    if (!status) {
        int32_t product_type = info.product_type;
        bool found = false;
        LOGD("%s: product_type: %d", __func__, product_type);

        for(int i = 0; module_quality_test_limits[i].product_type != -1; i++) {
            if(product_type == module_quality_test_limits[i].product_type) {
                found = true;
                *snr_threshold = module_quality_test_limits[i].snr_threshold;
                *snr_limit_preset = module_quality_test_limits[i].snr_limit_preset;
                *snr_cropping_left = module_quality_test_limits[i].snr_cropping_left;
                *snr_cropping_top = module_quality_test_limits[i].snr_cropping_top;
                *snr_cropping_right = module_quality_test_limits[i].snr_cropping_right;
                *snr_cropping_bottom = module_quality_test_limits[i].snr_cropping_bottom;
                *stablization_ms = STABILIZATION_MS;
                break;
            }
        }
        if (found) {
            // parse test limits
            char* key_context = NULL;
            char* key = strtok_r(module->test_input.test_limits_key_value_pair, ";", &key_context);
            char* value_context = NULL;
            char* value = NULL;
            for (int i = 0; (i < 4) && (key != NULL); i++) {
                if (!STRNCMP(key, "snr=")) {
                    value = key + strlen("snr=");
                    *snr_threshold = atof(value);
                    LOGD("%s: override snr_threshold: %f", __func__, *snr_threshold);
                } else if (!STRNCMP(key, "snrlimitpreset=")) {
                    value = key + strlen("snrlimitpreset=");
                    *snr_limit_preset = atoi(value);
                    LOGD("%s: override snr_limit_preset: %d", __func__, *snr_limit_preset);
                } else if (!STRNCMP(key, "snrcropping=")) {
                    value = strtok_r(key + strlen("snrcropping="), ",", &value_context);
                    for (int j = 0; (j < 4) && (value != NULL); j++) {
                        if (j == 0) {
                            *snr_cropping_left = atoi(value);
                        } else if (j == 1) {
                            *snr_cropping_top = atoi(value);
                        } else if (j == 2) {
                            *snr_cropping_right = atoi(value);
                        } else if (j == 3) {
                            *snr_cropping_bottom = atoi(value);
                        }
                        value = strtok_r(NULL, ",", &value_context);
                    }
                    LOGD("%s override snr_cropping left:%d top:%d right:%d bottom:%d ",
                         __func__,
                         *snr_cropping_left,
                         *snr_cropping_top,
                         *snr_cropping_right,
                         *snr_cropping_bottom);
                } else if (!STRNCMP(key, "stabilizationms=")) {
                    value = key + strlen("stabilizationms=");
                    *stablization_ms = atoi(value);
                    LOGD("%s: override stabilizationms: %d", __func__, *stablization_ms);
                }

                key = strtok_r(NULL, ";", &key_context);
            }
        } else {
            status = FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
        }
    }
    return status;
}

static void do_run_sensor_test(void* self)
{
    LOGD("%s", __func__);
    int status = FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED;
    uint32_t result = 0;
    sensortest_module_t* module = (sensortest_module_t*) self;

    LOGD("%s: %s start", __func__, module->test.name);

    fpc_hal_ext_sensortest_test_result_t* test_result =
        (fpc_hal_ext_sensortest_test_result_t*) malloc(sizeof(fpc_hal_ext_sensortest_test_result_t));

    memset(test_result, 0, sizeof(fpc_hal_ext_sensortest_test_result_t));

    if (test_result) {
        memset(test_result, 0, sizeof(fpc_hal_ext_sensortest_test_result_t));

        if (STRNCMP(module->test.name, SELF_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_SELF_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, CHECKERBOARD_TEST_TITLE) == 0) {
            test_result->image_fetched = true;
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_CHECKERBOARD_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, IMAGE_QUALITY_TEST_TITLE) == 0) {
            test_result->image_fetched = true;
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, IMAGE_RESET_PIXEL_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_RESET_PIXEL_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, AFD_CALIBRATION_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, AFD_CALIBRATION_RUBBER_STAMP_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, AFD_RUBBER_STAMP_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST,
                                                 &result);
        } else if (STRNCMP(module->test.name, MODULE_QUALITY_TEST_TITLE) == 0) {
            float snr_threshold = 0;
            uint32_t stablization_ms = 0;
            uint32_t snr_limit_preset = 0;
            uint32_t snr_cropping_left = 0;
            uint32_t snr_cropping_top = 0;
            uint32_t snr_cropping_right = 0;
            uint32_t snr_cropping_bottom = 0;

            status = get_module_quality_test_limit(module,
                                                   &snr_threshold,
                                                   &snr_limit_preset,
                                                   &snr_cropping_left,
                                                   &snr_cropping_top,
                                                   &snr_cropping_right,
                                                   &snr_cropping_bottom,
                                                   &stablization_ms);
            if (!status) {
                uint32_t snr_value;
                uint32_t snr_error;

                test_result->image_fetched = true;
                status = fpc_tee_sensortest_run_module_quality_test(module->hal->sensor,
                                                                    stablization_ms,
                                                                    snr_limit_preset,
                                                                    snr_cropping_left,
                                                                    snr_cropping_top,
                                                                    snr_cropping_right,
                                                                    snr_cropping_bottom,
                                                                    &result,
                                                                    &snr_value,
                                                                    &snr_error);
                if (!status) {
                    float snr = (float) snr_value / 100;
                    LOGD("%s: error:-%d snr: %f threshold: %f", __func__, snr_error, snr, snr_threshold);

                    if (result == FPC_TA_SENSORTEST_TEST_OK) {
#ifdef FPC_CONFIG_ENGINEERING
                        snprintf(test_result->result_string,
                                 FPC_HAL_EXT_SENSORTEST_TEST_RESULT_MAX,
                                 "SNR: %f (limit: >= %f)",
                                 snr,
                                 snr_threshold);
#endif
                        if (snr < snr_threshold) {
                            result = 1;
                        }
                    } else {
#ifdef FPC_CONFIG_ENGINEERING
                        snprintf(test_result->error_string,
                                 FPC_HAL_EXT_SENSORTEST_TEST_ERROR_MAX,
                                 "SNR Error:-%d",
                                 snr_error);
#endif
                    }
                }
            }
        } else if (STRNCMP(module->test.name, OTP_VALIDATION_TEST_TITLE) == 0) {
            fpc_hw_module_info_t info;
            status = fpc_tee_get_sensor_otp_info(module->hal->sensor, &info);
            if (!status && ((int)info.otp_error_info.total_num_bit_errors > 0)) {
                result = 1;
            }
        }  else if (STRNCMP(module->test.name, PN_IMAGE_TEST_TITLE) == 0) {
            status = fpc_tee_sensortest_run_test(module->hal->sensor,
                                                 FPC_TEE_SENSORTEST_PN_IMAGE_TEST,
                                                 &result);
        }
        LOGD("%s: %s finished with status: %d result: %d", __func__, module->test.name, status, result);

        if (status) {
            if (status == -FPC_ERROR_CANCELLED) {
                test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_CANCELLED;
            } else if (status == FPC_TA_SENSORTEST_TEST_NOT_SUPPORTED) {
                test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_NOT_SUPPORTED;
            } else {
                test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_ERROR;
            }
            test_result->error_code = status;
        } else {
            if (result == FPC_TA_SENSORTEST_TEST_OK) {
                test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_PASS;
            } else {
                test_result->result_code = FPC_HAL_EXT_SENSORTEST_TEST_FAIL;
            }
        }

#ifdef FPC_CONFIG_ENGINEERING
        if (test_result->image_fetched) {
            fpc_hal_ext_get_raw_image(module->engineering, &test_result->image_result);
        }
#else
        test_result->image_fetched = false;
#endif

        module->test_result_cb(module->test_cb_ctx, test_result);

#ifdef FPC_CONFIG_ENGINEERING
        if (test_result->image_fetched) {
            fpc_hal_ext_free_image(&test_result->image_result);
        }
#endif

        free(test_result);
        test_result = NULL;
    } else {
        LOGE("%s failed to allocate test_result", __func__);
        module->test_result_cb(module->test_cb_ctx, NULL);
    }

    module->test_cb_ctx = NULL;
    module->test_result_cb = NULL;
}

static int run_sensor_test(fpc_hal_ext_sensortest_t* self, fpc_hal_ext_sensortest_test_t* test,
        fpc_hal_ext_sensortest_test_input_t* input, void* ctx, fpc_test_result_cb_t result_cb)
{
    int status = 0;
    sensortest_module_t* module = (sensortest_module_t*) self;

    if (test == NULL) {
        LOGE("%s failed test is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (ctx == NULL) {
        LOGE("%s failed ctx is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (result_cb == NULL) {
        LOGE("%s failed result_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    LOGD("%s %s\n", __func__, test->name);

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    module->test = *test;
    if (input) {
        module->test_input = *input;
    }
    module->test_cb_ctx = ctx;
    module->test_result_cb = result_cb;

    fingerprint_hal_do_async_work(module->hal, do_run_sensor_test, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

out:
    return status;
}

static void cancel_sensor_test(fpc_hal_ext_sensortest_t* self)
{
    LOGD("%s", __func__);
    sensortest_module_t* module = (sensortest_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

static void do_capture(void* self)
{
    LOGD("%s", __func__);
    int status;
    sensortest_module_t* module = (sensortest_module_t*) self;

    if (module->uncalibrated) {
        status = fpc_tee_sensortest_capture_uncalibrated(module->hal->sensor);
        if (status) {
            goto out;
        }

    } else if (module->wait_for_finger) {
        for (;;) {
            status = fpc_tee_capture_image(module->hal->sensor);
            if ((status == FPC_CAPTURE_QUALIFY_ABORT) ||
                (status == FPC_CAPTURE_FINGER_STUCK)) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image or if finger is down
                // for a long time.
                continue;
            } else if (status == FPC_CAPTURE_FINGER_LOST) {
                LOGD("%s fpc_tee_capture_image finger removed too fast.", __func__);
            } else {
                goto out;
            }
        }
    } else {
        status = fpc_tee_capture_snapshot(module->hal->sensor);
        if (status) {
            LOGD("%s fpc_capture_image failed: %i \n", __func__, status);
            goto out;
        }
    }

out:
    // Send result callback
    switch (status) {
        case FPC_CAPTURE_OK:
            module->capture_acquired_cb(module->capture_cb_ctx,
                    HAL_COMPAT_ACQUIRED_GOOD);
            if (module->engineering) {
                module->engineering->handle_image_subscription(module->engineering);
            }
            break;
        case FPC_CAPTURE_BAD_QUALITY:
            module->capture_acquired_cb(module->capture_cb_ctx,
                    HAL_COMPAT_ACQUIRED_INSUFFICIENT);
            if (module->engineering) {
                module->engineering->handle_image_subscription(module->engineering);
            }
            break;
        case -EINTR:
        case -FPC_ERROR_IO:
        case -FPC_ERROR_CANCELLED:
            module->capture_error_cb(module->capture_cb_ctx,
                    HAL_COMPAT_ERROR_CANCELED);
            break;
        default:
            LOGD("%s status code = %d", __func__, status);
            break;
    }

    // Clear callback
    module->capture_cb_ctx = NULL;
    module->capture_acquired_cb = NULL;
    module->capture_error_cb = NULL;
}

static int capture(fpc_hal_ext_sensortest_t* self, bool wait_for_finger,
        bool uncalibrated, void* ctx, fpc_capture_acquired_cb_t acquired_cb,
        fpc_capture_error_cb_t error_cb)
{
    int status = 0;
    sensortest_module_t* module = (sensortest_module_t*) self;

    LOGD("%s wait_for_finger: %d, uncalibrated: %d\n", __func__, wait_for_finger, uncalibrated);

    if (ctx == NULL) {
        LOGE("%s failed ctx is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (acquired_cb == NULL) {
        LOGE("%s failed acquired_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (error_cb == NULL) {
        LOGE("%s failed error_cb is NULL\n", __func__);
        status = -EINVAL;
        goto out;
    }

    if (wait_for_finger && uncalibrated) {
        LOGE("%s wait_for_finger can not be used together with uncalibrated\n", __func__);
        status = -EINVAL;
        goto out;
    }

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);

    module->wait_for_finger = wait_for_finger;
    module->uncalibrated = false;
    module->capture_cb_ctx = ctx;
    module->capture_acquired_cb = acquired_cb;
    module->capture_error_cb = error_cb;

    fingerprint_hal_do_async_work(module->hal, do_capture, module, FPC_TASK_HAL_EXT);
    pthread_mutex_unlock(&module->hal->lock);

out:
    return status;
}

static void cancel_capture(fpc_hal_ext_sensortest_t* self)
{
    LOGD("%s", __func__);
    sensortest_module_t* module = (sensortest_module_t*) self;

    pthread_mutex_lock(&module->hal->lock);
    fingerprint_hal_goto_idle(module->hal);
    fingerprint_hal_resume(module->hal);
    pthread_mutex_unlock(&module->hal->lock);
}

fpc_hal_ext_sensortest_t* fpc_sensortest_new(fpc_hal_common_t* hal,
                                     fpc_engineering_t* engineering)
{
    sensortest_module_t* module = malloc(sizeof(sensortest_module_t));
    if (!module) {
        return NULL;
    }

    memset(module, 0, sizeof(sensortest_module_t));
    module->hal = hal;
    module->sensortest.get_sensor_info = get_sensor_info;
    module->sensortest.get_sensor_tests = get_sensor_tests;
    module->sensortest.run_sensor_test = run_sensor_test;
    module->sensortest.cancel_sensor_test = cancel_sensor_test;
    module->sensortest.capture = capture;
    module->sensortest.cancel_capture = cancel_capture;
    module->engineering = engineering;

    return (fpc_hal_ext_sensortest_t*) module;
}

void fpc_sensortest_destroy(fpc_hal_ext_sensortest_t *self)
{
    free(self);
}
