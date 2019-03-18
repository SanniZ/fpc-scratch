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

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::ICalibrationCallback;
using ::com::fingerprints::extension::V1_0::IFingerprintCalibration;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FingerprintCalibration : public IFingerprintCalibration {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintCalibration follow.
    Return<void> calibrate(const sp<ICalibrationCallback>& callback) override;

    static void instantiate(fpc_calibration_t* device);
private:
    FingerprintCalibration(fpc_calibration_t* device);
    static int onStatus(void* context, uint8_t code);
    static int onError(void* context, int8_t code);

    static FingerprintCalibration* sInstance;
    sp<ICalibrationCallback> mCalibrationCallback;
    fpc_calibration_t* mDevice;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTCALIBRATION_H
