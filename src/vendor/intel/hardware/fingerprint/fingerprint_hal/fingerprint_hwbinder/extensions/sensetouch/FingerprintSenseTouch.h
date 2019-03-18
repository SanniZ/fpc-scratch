#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H

#include <com/fingerprints/extension/1.0/IFingerprintSenseTouch.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_sense_touch_service.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFingerprintSenseTouch;
using ::com::fingerprints::extension::V1_0::SenseTouchConfig;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct FingerprintSenseTouch : public IFingerprintSenseTouch {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintSenseTouch follow.
    Return<uint32_t> getForce() override;
    Return<bool> isSupported() override;
    Return<bool> finishCalibration(uint32_t ground, uint32_t threshold) override;
    Return<bool> setAuthMode(SenseTouchAuthenticationMode mode, uint32_t buttonTimeoutMs) override;
    Return<void> readConfig(readConfig_cb _hidl_cb) override;

    static void instantiate(fpc_sense_touch_t* device);
private:
   FingerprintSenseTouch(fpc_sense_touch_t* device);
   virtual ~FingerprintSenseTouch();
   static FingerprintSenseTouch* sInstance;
   fpc_sense_touch_t* mDevice;
};

extern "C" IFingerprintSenseTouch* HIDL_FETCH_IFingerprintSenseTouch(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTSENSETOUCH_H
