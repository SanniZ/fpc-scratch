#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H

#include <com/fingerprints/extension/1.0/IFingerprintSenseTouch.h>
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

struct FingerprintSenseTouch : public IFingerprintSenseTouch {
    // Methods from IFingerprintSenseTouch follow.
    Return<uint32_t> getForce() override;
    Return<bool> isSupported() override;
    Return<bool> finishCalibration(uint32_t ground, uint32_t threshold) override;
    Return<bool> setAuthMode(SenseTouchAuthenticationMode mode, uint32_t buttonTimeoutMs) override;
    Return<void> readConfig(readConfig_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintSenseTouch* HIDL_FETCH_IFingerprintSenseTouch(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H
