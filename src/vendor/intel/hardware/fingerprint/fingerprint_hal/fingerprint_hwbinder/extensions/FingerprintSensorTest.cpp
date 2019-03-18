#include "FingerprintSensorTest.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintSensorTest follow.
Return<void> FingerprintSensorTest::getSensorInfo(getSensorInfo_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> FingerprintSensorTest::getSensorTests(getSensorTests_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> FingerprintSensorTest::runSensorTest(const sp<ISensorTestCallback>& callback, const SensorTest& test, const SensorTestInput& input) {
    // TODO implement
    return Void();
}

Return<void> FingerprintSensorTest::cancelSensorTest() {
    // TODO implement
    return Void();
}

Return<void> FingerprintSensorTest::capture(const sp<ISensorTestCaptureCallback>& callback, bool waitForFinger, bool uncalibrated) {
    // TODO implement
    return Void();
}

Return<void> FingerprintSensorTest::cancelCapture() {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintSensorTest* HIDL_FETCH_IFingerprintSensorTest(const char* /* name */) {
//    return new FingerprintSensorTest();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
