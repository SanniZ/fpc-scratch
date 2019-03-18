#include "FingerprintAuthenticator.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IFingerprintAuthenticator follow.
Return<uint32_t> FingerprintAuthenticator::verifyUser(const sp<IVerifyUserCallback>& callback, const hidl_vec<uint8_t>& nonce, const hidl_vec<uint8_t>& dstAppName) {
    // TODO implement
    return uint32_t {};
}

Return<bool> FingerprintAuthenticator::isUserValid(uint64_t userId) {
    // TODO implement
    return bool {};
}

Return<void> FingerprintAuthenticator::cancel() {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IFingerprintAuthenticator* HIDL_FETCH_IFingerprintAuthenticator(const char* /* name */) {
//    return new FingerprintAuthenticator();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
