#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H

#include <com/fingerprints/extension/1.0/IFingerprintEngineering.h>
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

struct FingerprintEngineering : public IFingerprintEngineering {
    // Methods from IFingerprintEngineering follow.
    Return<void> getSensorSize(getSensorSize_cb _hidl_cb) override;
    Return<void> startImageSubscription(const sp<IImageCaptureCallback>& callback) override;
    Return<void> stopImageSubscription() override;
    Return<void> startImageInjection(const sp<IImageInjectionCallback>& callback) override;
    Return<void> stopImageInjection() override;
    Return<void> startCapture(const sp<IImageCaptureCallback>& callback, uint32_t mode) override;
    Return<void> cancelCapture() override;
    Return<void> setEnrollToken(const hidl_vec<uint8_t>& token) override;
    Return<uint64_t> getEnrollChallenge() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintEngineering* HIDL_FETCH_IFingerprintEngineering(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H
