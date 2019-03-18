#include "SensorTestCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from ISensorTestCallback follow.
Return<void> SensorTestCallback::onResult(const SensorTestResult& result) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISensorTestCallback* HIDL_FETCH_ISensorTestCallback(const char* /* name */) {
//    return new SensorTestCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
