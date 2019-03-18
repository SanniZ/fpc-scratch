#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H

#include <com/fingerprints/extension/1.0/IFingerprintAuthenticator.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_authenticator_service.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFingerprintAuthenticator;
using ::com::fingerprints::extension::V1_0::IVerifyUserCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

struct FingerprintAuthenticator : public IFingerprintAuthenticator, public hidl_death_recipient {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintAuthenticator follow.
    Return<uint32_t> verifyUser(const sp<IVerifyUserCallback>& callback,
                                const hidl_vec<uint8_t>& nonce,
                                const hidl_vec<uint8_t>& dstAppName) override;
    Return<bool> isUserValid(uint64_t userId) override;
    Return<void> cancel() override;

    static void instantiate(fpc_authenticator_t* device);
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

private:
   static void onResult(void* context, fpc_verify_user_data_t verify_user_data);
   static void onHelp(void* context, int32_t helpCode);
   FingerprintAuthenticator(fpc_authenticator_t* device);

   static FingerprintAuthenticator* sInstance;
   sp<IVerifyUserCallback> mVerifyUserCallback;
   fpc_authenticator_t* mDevice;
   bool mVerifyUser;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTAUTHENTICATOR_H
