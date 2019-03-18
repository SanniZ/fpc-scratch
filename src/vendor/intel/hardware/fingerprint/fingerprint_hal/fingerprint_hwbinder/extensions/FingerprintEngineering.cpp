#include "FingerprintEngineering.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintEngineering follow.
Return<void> FingerprintEngineering::getSensorSize(getSensorSize_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::startImageSubscription(const sp<IImageCaptureCallback>& callback) {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::stopImageSubscription() {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::startImageInjection(const sp<IImageInjectionCallback>& callback) {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::stopImageInjection() {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::startCapture(const sp<IImageCaptureCallback>& callback, uint32_t mode) {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::cancelCapture() {
    // TODO implement
    return Void();
}

Return<void> FingerprintEngineering::setEnrollToken(const hidl_vec<uint8_t>& token) {
    // TODO implement
    return Void();
}

Return<uint64_t> FingerprintEngineering::getEnrollChallenge() {
    // TODO implement
    return uint64_t {};
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintEngineering* HIDL_FETCH_IFingerprintEngineering(const char* /* name */) {
//    return new FingerprintEngineering();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
