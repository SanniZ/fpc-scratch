#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H

#include <com/fingerprints/extension/1.0/IFingerprintAuthenticator.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FingerprintAuthenticator : public IFingerprintAuthenticator {
    // Methods from IFingerprintAuthenticator follow.
    Return<uint32_t> verifyUser(const sp<IVerifyUserCallback>& callback, const hidl_vec<uint8_t>& nonce, const hidl_vec<uint8_t>& dstAppName) override;
    Return<bool> isUserValid(uint64_t userId) override;
    Return<void> cancel() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IFingerprintAuthenticator* HIDL_FETCH_IFingerprintAuthenticator(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H
