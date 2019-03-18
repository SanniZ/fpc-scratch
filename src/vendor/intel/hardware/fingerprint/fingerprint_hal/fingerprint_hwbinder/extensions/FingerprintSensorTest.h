#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H

#include <com/fingerprints/extension/1.0/IFingerprintSensorTest.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FingerprintSensorTest : public IFingerprintSensorTest {
    // Methods from IFingerprintSensorTest follow.
    Return<void> getSensorInfo(getSensorInfo_cb _hidl_cb) override;
    Return<void> getSensorTests(getSensorTests_cb _hidl_cb) override;
    Return<void> runSensorTest(const sp<ISensorTestCallback>& callback, const SensorTest& test, const SensorTestInput& input) override;
    Return<void> cancelSensorTest() override;
    Return<void> capture(const sp<ISensorTestCaptureCallback>& callback, bool waitForFinger, bool uncalibrated) override;
    Return<void> cancelCapture() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintSensorTest* HIDL_FETCH_IFingerprintSensorTest(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSORTEST_H
