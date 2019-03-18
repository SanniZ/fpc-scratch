#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTCALIBRATION_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTCALIBRATION_H

#include <com/fingerprints/extension/1.0/IFingerprintCalibration.h>
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

struct FingerprintCalibration : public IFingerprintCalibration {
    // Methods from IFingerprintCalibration follow.
    Return<void> calibrate(const sp<ICalibrationCallback>& callback) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintCalibration* HIDL_FETCH_IFingerprintCalibration(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTCALIBRATION_H
