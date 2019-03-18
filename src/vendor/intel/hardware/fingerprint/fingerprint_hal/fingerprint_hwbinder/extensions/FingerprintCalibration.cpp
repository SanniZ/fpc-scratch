#include "FingerprintCalibration.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintCalibration follow.
Return<void> FingerprintCalibration::calibrate(const sp<ICalibrationCallback>& callback) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintCalibration* HIDL_FETCH_IFingerprintCalibration(const char* /* name */) {
//    return new FingerprintCalibration();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
