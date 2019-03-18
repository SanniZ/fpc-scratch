#include "VerifyUserCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IVerifyUserCallback follow.
Return<void> VerifyUserCallback::onResult(uint32_t result, uint64_t userId, uint64_t userEntityId, const hidl_vec<uint8_t>& encapsulatedResult) {
    // TODO implement
    return Void();
}

Return<void> VerifyUserCallback::onHelp(uint32_t helpCode) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IVerifyUserCallback* HIDL_FETCH_IVerifyUserCallback(const char* /* name */) {
//    return new VerifyUserCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
