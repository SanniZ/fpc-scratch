#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTNAVIGATION_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTNAVIGATION_H

#include <com/fingerprints/extension/1.0/IFingerprintNavigation.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_navigation.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFingerprintNavigation;
using ::com::fingerprints::extension::V1_0::NavigationConfig;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FingerprintNavigation : public IFingerprintNavigation {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintNavigation follow.
    Return<void> setNavigation(bool enabled) override;
    Return<void> getNavigationConfig(getNavigationConfig_cb _hidl_cb) override;
    Return<void> setNavigationConfig(const NavigationConfig& navigationConfig) override;
    Return<bool> isEnabled() override;

    static void instantiate(fpc_navigation_t* device);

private:
    FingerprintNavigation(fpc_navigation_t* device);
    virtual ~FingerprintNavigation();

    static FingerprintNavigation* sInstance;
    fpc_navigation_t* mDevice;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTNAVIGATION_H
