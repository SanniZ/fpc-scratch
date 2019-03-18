/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_SENSORTEST_H_
#define FINGERPRINT_SENSORTEST_H_

#include "IFingerprintSensorTest.h"
#include "fpc_hal_ext_sensortest.h"

namespace android {

class FingerprintSensorTest : public BnFingerprintSensorTest,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_hal_ext_sensortest_t* device);
        static void testOnResult(void* context,
                                 fpc_hal_ext_sensortest_test_result_t* result);
        static void captureOnAcquired(void* context, int32_t acquiredInfo);
        static void captureOnError(void* context, int32_t error);

        // These reflect binder methods.
        virtual void getSensorInfo(fpc_hw_module_info_t* info);
        virtual void getSensorTests(
                fpc_hal_ext_sensortest_tests_t* sensorTests);
        virtual void runSensorTest(const sp<ISensorTestCallback>& callback,
                                   fpc_hal_ext_sensortest_test_t* test,
                                   fpc_hal_ext_sensortest_test_input_t* input);
        virtual void cancelSensorTest();
        virtual void capture(const sp<ISensorTestCaptureCallback>& callback,
                             bool waitForFinger, bool uncalibrated);
        virtual void cancelCapture();

     private:
        FingerprintSensorTest(fpc_hal_ext_sensortest_t* device);
        virtual ~FingerprintSensorTest();
        void binderDied(const wp<IBinder>& who);

        static FingerprintSensorTest* sInstance;
        sp<ISensorTestCallback> mSensorTestCallback;
        sp<ISensorTestCaptureCallback> mCaptureCallback;
        fpc_hal_ext_sensortest_t* mDevice;
        bool mSensorTest;
        bool mCapture;
};

}  // namespace android

#endif // FINGERPRINT_SENSORTEST_H_
