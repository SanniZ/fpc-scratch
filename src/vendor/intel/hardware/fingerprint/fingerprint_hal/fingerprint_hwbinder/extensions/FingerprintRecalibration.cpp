#include "FingerprintRecalibration.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintRecalibration follow.
Return<void> FingerprintRecalibration::recalibrate(const hidl_vec<uint8_t>& token, const sp<IRecalibrationCallback>& callback) {
    // TODO implement
    return Void();
}

Return<uint64_t> FingerprintRecalibration::preRecalibrate() {
    // TODO implement
    return uint64_t {};
}

Return<void> FingerprintRecalibration::cancel() {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintRecalibration* HIDL_FETCH_IFingerprintRecalibration(const char* /* name */) {
//    return new FingerprintRecalibration();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
