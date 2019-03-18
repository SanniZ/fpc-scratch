#include "FingerprintSenseTouch.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintSenseTouch follow.
Return<uint32_t> FingerprintSenseTouch::getForce() {
    // TODO implement
    return uint32_t {};
}

Return<bool> FingerprintSenseTouch::isSupported() {
    // TODO implement
    return bool {};
}

Return<bool> FingerprintSenseTouch::finishCalibration(uint32_t ground, uint32_t threshold) {
    // TODO implement
    return bool {};
}

Return<bool> FingerprintSenseTouch::setAuthMode(SenseTouchAuthenticationMode mode, uint32_t buttonTimeoutMs) {
    // TODO implement
    return bool {};
}

Return<void> FingerprintSenseTouch::readConfig(readConfig_cb _hidl_cb) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintSenseTouch* HIDL_FETCH_IFingerprintSenseTouch(const char* /* name */) {
//    return new FingerprintSenseTouch();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
