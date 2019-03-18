#include "RecalibrationCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IRecalibrationCallback follow.
Return<void> RecalibrationCallback::onStatus(uint32_t code, bool imageDecision, uint32_t imageQuality, uint32_t pnQuality, uint32_t progress) {
    // TODO implement
    return Void();
}

Return<void> RecalibrationCallback::onError(uint32_t error) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IRecalibrationCallback* HIDL_FETCH_IRecalibrationCallback(const char* /* name */) {
//    return new RecalibrationCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
