#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H

#include <com/fingerprints/extension/1.0/IFingerprintRecalibration.h>
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

struct FingerprintRecalibration : public IFingerprintRecalibration {
    // Methods from IFingerprintRecalibration follow.
    Return<void> recalibrate(const hidl_vec<uint8_t>& token, const sp<IRecalibrationCallback>& callback) override;
    Return<uint64_t> preRecalibrate() override;
    Return<void> cancel() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintRecalibration* HIDL_FETCH_IFingerprintRecalibration(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H
