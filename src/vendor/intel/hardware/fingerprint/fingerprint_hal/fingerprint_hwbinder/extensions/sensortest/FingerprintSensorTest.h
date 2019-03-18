#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H

#include <com/fingerprints/extension/1.0/IFingerprintSensorTest.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_sensortest.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::ISensorTestCaptureCallback;
using ::com::fingerprints::extension::V1_0::IFingerprintSensorTest;
using ::com::fingerprints::extension::V1_0::ISensorTestCallback;
using ::com::fingerprints::extension::V1_0::SensorInfo;
using ::com::fingerprints::extension::V1_0::SensorTest;
using ::com::fingerprints::extension::V1_0::SensorTestInput;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

struct FingerprintSensorTest : public IFingerprintSensorTest, public hidl_death_recipient {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintSensorTest follow.
    Return<void> getSensorInfo(getSensorInfo_cb _hidl_cb) override;
    Return<void> getSensorTests(getSensorTests_cb _hidl_cb) override;
    Return<void> runSensorTest(const sp<ISensorTestCallback>& callback,
                               const SensorTest& test,
                               const SensorTestInput& input) override;
    Return<void> cancelSensorTest() override;
    Return<void> capture(const sp<ISensorTestCaptureCallback>& callback,
                         bool waitForFinger,
                         bool uncalibrated) override;
    Return<void> cancelCapture() override;
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

public:
    static void instantiate(fpc_hal_ext_sensortest_t* device);

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    FingerprintSensorTest(fpc_hal_ext_sensortest_t* device);

    static void testOnResult(void* context, fpc_hal_ext_sensortest_test_result_t* result);
    static void captureOnAcquired(void* context, int32_t acquiredInfo);
    static void captureOnError(void* context, int32_t error);

    static FingerprintSensorTest* sInstance;
    sp<ISensorTestCallback> mSensorTestCallback;
    sp<ISensorTestCaptureCallback> mCaptureCallback;
    fpc_hal_ext_sensortest_t* mDevice;
    bool mSensorTest;
    bool mCapture;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H
