#include "SensorTestCaptureCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from ISensorTestCaptureCallback follow.
Return<void> SensorTestCaptureCallback::onAcquired(uint32_t acquiredInfo) {
    // TODO implement
    return Void();
}

Return<void> SensorTestCaptureCallback::onError(uint32_t error) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISensorTestCaptureCallback* HIDL_FETCH_ISensorTestCaptureCallback(const char* /* name */) {
//    return new SensorTestCaptureCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
