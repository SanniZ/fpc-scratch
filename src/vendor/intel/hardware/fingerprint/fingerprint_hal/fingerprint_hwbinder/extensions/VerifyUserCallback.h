#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_VERIFYUSERCALLBACK_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_VERIFYUSERCALLBACK_H

#include <com/fingerprints/extension/1.0/IVerifyUserCallback.h>
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

struct VerifyUserCallback : public IVerifyUserCallback {
    // Methods from IVerifyUserCallback follow.
    Return<void> onResult(uint32_t result, uint64_t userId, uint64_t userEntityId, const hidl_vec<uint8_t>& encapsulatedResult) override;
    Return<void> onHelp(uint32_t helpCode) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IVerifyUserCallback* HIDL_FETCH_IVerifyUserCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_VERIFYUSERCALLBACK_H
