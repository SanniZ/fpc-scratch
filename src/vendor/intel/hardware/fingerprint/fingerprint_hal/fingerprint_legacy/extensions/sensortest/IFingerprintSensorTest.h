/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_SENSORTEST_H_
#define IFINGERPRINT_SENSORTEST_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_sensortest.h"
#include "fpc_hw_identification_types.h"

namespace android {

class ISensorTestCallback : public IInterface {
     public:
        // must be kept in sync with ISensorTestCallback.aidl
        enum {
            ON_RESULT = IBinder::FIRST_CALL_TRANSACTION,
        };

        virtual void onResult(fpc_hal_ext_sensortest_test_result_t* result) = 0;

        DECLARE_META_INTERFACE (SensorTestCallback);
};

class ISensorTestCaptureCallback : public IInterface {
     public:
        // must be kept in sync with ICaptureCallback.aidl
        enum {
            ON_ACQUIRED = IBinder::FIRST_CALL_TRANSACTION,
            ON_ERROR,
        };

        virtual void onAcquired(int32_t acquiredInfo) = 0;
        virtual void onError(int32_t error) = 0;

        DECLARE_META_INTERFACE (SensorTestCaptureCallback);
};

class IFingerprintSensorTest : public IInterface {
     public:
        // must be kept in sync with IFingerprintSensorTest.aidl
        enum {
            GET_SENSOR_INFO = IBinder::FIRST_CALL_TRANSACTION,
            GET_SENSOR_TESTS,
            RUN_SENSOR_TEST,
            CANCEL_SENSOR_TEST,
            CAPTURE,
            CANCEL_CAPTURE
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void getSensorInfo(fpc_hw_module_info_t* info) = 0;
        virtual void getSensorTests(
                fpc_hal_ext_sensortest_tests_t* sensorTests) = 0;
        virtual void runSensorTest(
                const sp<ISensorTestCallback>& callback,
                fpc_hal_ext_sensortest_test_t* test,
                fpc_hal_ext_sensortest_test_input_t* input) = 0;
        virtual void cancelSensorTest() = 0;
        virtual void capture(const sp<ISensorTestCaptureCallback>& callback,
                             bool waitForFinger, bool uncalibrated) = 0;
        virtual void cancelCapture() = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintSensorTest : public BnInterface<IFingerprintSensorTest> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);

     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_SENSORTEST_H_

