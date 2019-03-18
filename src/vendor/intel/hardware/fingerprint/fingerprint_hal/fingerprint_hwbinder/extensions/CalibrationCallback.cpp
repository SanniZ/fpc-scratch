#include "CalibrationCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from ICalibrationCallback follow.
Return<void> CalibrationCallback::onStatus(int32_t code) {
    // TODO implement
    return Void();
}

Return<void> CalibrationCallback::onError(int32_t error) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ICalibrationCallback* HIDL_FETCH_ICalibrationCallback(const char* /* name */) {
//    return new CalibrationCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
