#include "FingerprintNavigation.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintNavigation follow.
Return<void> FingerprintNavigation::setNavigation(bool enabled) {
    // TODO implement
    return Void();
}

Return<void> FingerprintNavigation::getNavigationConfig(getNavigationConfig_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> FingerprintNavigation::setNavigationConfig(const NavigationConfig& navigationConfig) {
    // TODO implement
    return Void();
}

Return<bool> FingerprintNavigation::isEnabled() {
    // TODO implement
    return bool {};
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintNavigation* HIDL_FETCH_IFingerprintNavigation(const char* /* name */) {
//    return new FingerprintNavigation();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
