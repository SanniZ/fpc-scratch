/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_sensortest_service.h"
#include "FingerprintSensorTest.h"
#include <android/log.h>

void add_sensortest_service(fpc_hal_ext_sensortest_t* device) {
    android::FingerprintSensorTest::instantiate(device);
}

namespace android {

FingerprintSensorTest* FingerprintSensorTest::sInstance = NULL;

void FingerprintSensorTest::instantiate(fpc_hal_ext_sensortest_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintSensorTest(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintSensorTest::FingerprintSensorTest(fpc_hal_ext_sensortest_t* device)
        : mSensorTestCallback(NULL),
          mCaptureCallback(NULL) {
    mDevice = device;
    mSensorTest = false;
    mCapture = false;
}

FingerprintSensorTest::~FingerprintSensorTest() {
}

void FingerprintSensorTest::binderDied(const wp<IBinder>& who) {
    if (IInterface::asBinder(mSensorTestCallback) == who) {
        mSensorTestCallback = NULL;
    } else if (IInterface::asBinder(mCaptureCallback) == who) {
        mCaptureCallback = NULL;
    }
}

void FingerprintSensorTest::testOnResult(
        void* context, fpc_hal_ext_sensortest_test_result_t* result) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mSensorTestCallback != NULL) {
        self->mSensorTest = false;
        self->mSensorTestCallback->onResult(result);
    }
}

void FingerprintSensorTest::captureOnAcquired(void* context,
                                              int32_t acquiredInfo) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mCaptureCallback != NULL) {
        self->mCapture = false;
        self->mCaptureCallback->onAcquired(acquiredInfo);
    }
}

void FingerprintSensorTest::captureOnError(void* context, int32_t error) {
    FingerprintSensorTest * self = static_cast<FingerprintSensorTest*>(context);
    if (self->mCaptureCallback != NULL) {
        self->mCapture = false;
        self->mCaptureCallback->onError(error);
    }
}

void FingerprintSensorTest::getSensorInfo(fpc_hw_module_info_t* info) {
    if (mDevice) {
        mDevice->get_sensor_info(mDevice, info);
    }
}

void FingerprintSensorTest::getSensorTests(
        fpc_hal_ext_sensortest_tests_t* sensorTests) {
    if (mDevice) {
        mDevice->get_sensor_tests(mDevice, sensorTests);
    }
}

void FingerprintSensorTest::runSensorTest(
        const sp<ISensorTestCallback>& callback,
        fpc_hal_ext_sensortest_test_t* test,
        fpc_hal_ext_sensortest_test_input_t* input) {
    if (mSensorTestCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mSensorTestCallback)) {
        IInterface::asBinder(mSensorTestCallback)->unlinkToDeath(this);
    }
    IInterface::asBinder(callback)->linkToDeath(this);
    mSensorTestCallback = callback;
    if (mDevice) {
        uint32_t status = mDevice->run_sensor_test(
                mDevice, test, input, this,
                FingerprintSensorTest::testOnResult);
        if (status == 0) {
            mSensorTest = true;
        }
    }
}

void FingerprintSensorTest::cancelSensorTest() {
    if (mSensorTestCallback != NULL) {
        IInterface::asBinder(mSensorTestCallback)->unlinkToDeath(this);
    }
    mSensorTestCallback = NULL;
    if (mDevice && mSensorTest) {
        mDevice->cancel_sensor_test(mDevice);
    }
}

void FingerprintSensorTest::capture(
        const sp<ISensorTestCaptureCallback>& callback, bool waitForFinger,
        bool uncalibrated) {
    if (mCaptureCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mCaptureCallback)) {
        IInterface::asBinder(mCaptureCallback)->unlinkToDeath(this);
    }
    IInterface::asBinder(callback)->linkToDeath(this);
    mCaptureCallback = callback;
    if (mDevice) {
        uint32_t status = mDevice->capture(
                mDevice, waitForFinger, uncalibrated, this,
                FingerprintSensorTest::captureOnAcquired,
                FingerprintSensorTest::captureOnError);
        if (status == 0) {
            mCapture = true;
        }
    }
}

void FingerprintSensorTest::cancelCapture() {
    if (mCaptureCallback != NULL) {
        IInterface::asBinder(mCaptureCallback)->unlinkToDeath(this);
    }
    mCaptureCallback = NULL;
    if (mDevice && mCapture) {
        mDevice->cancel_capture(mDevice);
    }
}

}
