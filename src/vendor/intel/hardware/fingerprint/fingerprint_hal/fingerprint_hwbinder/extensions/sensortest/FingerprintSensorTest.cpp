#include "FingerprintSensorTest.h"
#include "string.h"
#include "fpc_hal_ext_sensortest_service.h"

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/StrongPointer.h>
#include <binder/IPCThreadState.h>

void add_sensortest_service(fpc_hal_ext_sensortest_t* device) {
    ALOGE("add_sensortest_service");
    com::fingerprints::extension::V1_0::implementation::FingerprintSensorTest::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::std::vector;

FingerprintSensorTest* FingerprintSensorTest::sInstance = NULL;

void FingerprintSensorTest::instantiate(fpc_hal_ext_sensortest_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintSensorTest(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintSensorTest");
        }
    }
}

FingerprintSensorTest::FingerprintSensorTest(fpc_hal_ext_sensortest_t* device)
        : mSensorTestCallback(NULL),
          mCaptureCallback(NULL),
          mDevice(device),
          mSensorTest(false),
          mCapture(false) {
}

void FingerprintSensorTest::testOnResult(
        void* context, fpc_hal_ext_sensortest_test_result_t* result) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mSensorTestCallback != NULL) {
        self->mSensorTest = false;
        SensorTestResult res;
        res.resultCode = result->result_code;
        res.resultString = hidl_string(result->result_string);
        res.errorCode = result->error_code;
        res.errorString = hidl_string(result->error_string);
        hidl_vec<uint8_t> imgData;
        if (result->image_fetched) {
            imgData.setToExternal(result->image_result.buffer,
                    result->image_result.buffer_size);
        }
        res.imageData = imgData;

        if (!self->mSensorTestCallback->onResult(res).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintSensorTest::captureOnAcquired(void* context,
                                              int32_t acquiredInfo) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mCaptureCallback != NULL) {
        self->mCapture = false;
        if (!self->mCaptureCallback->onAcquired(acquiredInfo).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintSensorTest::captureOnError(void* context, int32_t error) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mCaptureCallback != NULL) {
        self->mCapture = false;
        if (!self->mCaptureCallback->onError(error).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintSensorTest::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    (void)cookie;
    if (who == mSensorTestCallback) {
        cancelSensorTest();
        mCaptureCallback = NULL;
    } else if (who == mCaptureCallback) {
        cancelCapture();
    }
}

Return<void> FingerprintSensorTest::getSensorInfo(getSensorInfo_cb _hidl_cb) {
    if (mDevice) {
        SensorInfo sensorInfo;

        fpc_hw_module_info_t info = {};
        info.sensor_die_info.hardware_id = INVALID_HARDWARE_ID;
        info.sensor_die_info.lot_id[0] = INVALID_LOT_ID;
        info.sensor_die_info.wafer_id = INVALID_WAFTER_ID;
        info.sensor_die_info.wafer_position_x = INVALID_WAFER_POSITION;
        info.sensor_die_info.wafer_position_y = INVALID_WAFER_POSITION;
        info.sensor_die_info.production_timestamp[0] = INVALID_PROD_TIMESTAMP;
        info.companion_die_info.hardware_id = INVALID_HARDWARE_ID;
        info.companion_die_info.lot_id[0] = INVALID_LOT_ID;

        mDevice->get_sensor_info(mDevice, &info);
        sensorInfo.hardwareId = info.sensor_die_info.hardware_id;
        sensorInfo.sensorDieValidFlag = info.sensor_die_info.valid_field;
        sensorInfo.hardwareId = info.sensor_die_info.hardware_id;
        sensorInfo.lotId = hidl_string(info.sensor_die_info.lot_id);
        sensorInfo.waferId = info.sensor_die_info.wafer_id;
        sensorInfo.waferPositionX = info.sensor_die_info.wafer_position_x;
        sensorInfo.waferPositionY = info.sensor_die_info.wafer_position_y;
        sensorInfo.productionTimestamp = hidl_string(info.sensor_die_info.production_timestamp);
        sensorInfo.companionDieValidFlag = info.companion_die_info.valid_field;
        sensorInfo.companionChipHardwareId = info.companion_die_info.hardware_id;
        sensorInfo.companionChipLotId = hidl_string(info.companion_die_info.lot_id);
        sensorInfo.vendorHwValidFlag = info.vendor_otp_info.valid_field;

        hidl_vec<uint8_t> data;
        data.setToExternal(info.vendor_otp_info.vendor_data, info.vendor_otp_info.vendor_data_size);
        sensorInfo.vendorData = data;

        sensorInfo.totalNumOtpBitErrors = info.otp_error_info.total_num_bit_errors;
        sensorInfo.maxNumOtpBitErrorsInByte = info.otp_error_info.max_num_bit_errors_in_byte;
        sensorInfo.productType = info.product_type;
        _hidl_cb(sensorInfo);
    }

    return Void();
}

Return<void> FingerprintSensorTest::getSensorTests(getSensorTests_cb _hidl_cb) {
    hidl_vec < SensorTest > result;
    if (mDevice) {
        fpc_hal_ext_sensortest_tests_t* sensorTests = (fpc_hal_ext_sensortest_tests_t*) calloc(
                1, sizeof(fpc_hal_ext_sensortest_tests_t));
        if (sensorTests) {
            mDevice->get_sensor_tests(mDevice, sensorTests);
            if (sensorTests->size > 0) {
                vector <SensorTest> tests(sensorTests->size);
                for (uint32_t i = 0; i < sensorTests->size; i++) {
                    SensorTest test;
                    test.name = hidl_string(sensorTests->tests[i].name);
                    test.description = hidl_string(sensorTests->tests[i].description);
                    test.waitForFingerDown = sensorTests->tests[i].wait_for_finger_down ? 1 : 0;
                    test.rubberStampType = hidl_string(sensorTests->tests[i].rubber_stamp_type);
                    tests[i] = test;
                }
                result = tests;
            }
            free(sensorTests);
        }
    }
    _hidl_cb(result);
    return Void();
}

Return<void> FingerprintSensorTest::runSensorTest(const sp<ISensorTestCallback>& callback,
                                                  const SensorTest& test,
                                                  const SensorTestInput& input) {
    if (mSensorTestCallback != NULL) {
        mSensorTestCallback->unlinkToDeath(this);
    }
    mSensorTestCallback = callback;
    if (mSensorTestCallback != NULL) {
        mSensorTestCallback->linkToDeath(this, 0);
    }
    fpc_hal_ext_sensortest_test_t* nativeTest = (fpc_hal_ext_sensortest_test_t*)
            calloc(1, sizeof(fpc_hal_ext_sensortest_test_t));
    fpc_hal_ext_sensortest_test_input_t* nativeInput = (fpc_hal_ext_sensortest_test_input_t*)
            calloc(1, sizeof(fpc_hal_ext_sensortest_test_input_t));
    if (mDevice && nativeTest && nativeInput) {
        if (strlcpy(nativeTest->name, test.name.c_str(), FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX)
                >= FPC_HAL_EXT_SENSORTEST_TEST_NAME_MAX) {
            ALOGE("Test name truncated");
            goto out;
        }
        if (strlcpy(nativeTest->description, test.description.c_str(),
                    FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX)
                >= FPC_HAL_EXT_SENSORTEST_TEST_DESCRIPTION_MAX) {
            ALOGE("Test description truncated");
            goto out;
        }
        nativeTest->wait_for_finger_down = test.waitForFingerDown;
        if (nativeInput) {
            if (strlcpy(nativeInput->test_limits_key_value_pair,
                        input.testLimitsKeyValuePair.c_str(),
                        FPC_HAL_EXT_SENSORTEST_TEST_INPUT_MAX)
                    >= FPC_HAL_EXT_SENSORTEST_TEST_INPUT_MAX) {
                ALOGE("Test limits truncated");
                goto out;
            }
            uint32_t status = mDevice->run_sensor_test(mDevice, nativeTest, nativeInput,
                                                       this, FingerprintSensorTest::testOnResult);
            if (status == 0) {
                mSensorTest = true;
            }

        }
    }
out:
    free(nativeTest);
    free(nativeInput);
    return Void();
}

Return<void> FingerprintSensorTest::cancelSensorTest() {
    if (mSensorTestCallback != NULL) {
        mSensorTestCallback->unlinkToDeath(this);
        mSensorTestCallback = NULL;
    }

    if (mDevice && mSensorTest) {
        mDevice->cancel_sensor_test(mDevice);
    }
    return Void();
}

Return<void> FingerprintSensorTest::capture(const sp<ISensorTestCaptureCallback>& callback,
                                            bool waitForFinger,
                                            bool uncalibrated) {
    if (mCaptureCallback != NULL) {
        mCaptureCallback->unlinkToDeath(this);
    }
    mCaptureCallback = callback;
    if (mCaptureCallback != NULL) {
        mCaptureCallback->linkToDeath(this, 0);
    }
    if (mDevice) {
        uint32_t status = mDevice->capture(
                mDevice, waitForFinger, uncalibrated, this,
                FingerprintSensorTest::captureOnAcquired,
                FingerprintSensorTest::captureOnError);
        if (status == 0) {
            mCapture = true;
        }
    }
    return Void();
}

Return<void> FingerprintSensorTest::cancelCapture() {
    if (mCaptureCallback != NULL) {
        mCaptureCallback->unlinkToDeath(this);
        mCaptureCallback = NULL;
    }

    if (mDevice && mCapture) {
        mDevice->cancel_capture(mDevice);
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
