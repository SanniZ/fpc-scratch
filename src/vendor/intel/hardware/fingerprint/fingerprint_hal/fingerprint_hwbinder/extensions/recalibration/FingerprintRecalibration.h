#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H

#include "fpc_hal_ext_recalibration_service.h"
#include <com/fingerprints/extension/1.0/IFingerprintRecalibration.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFingerprintRecalibration;
using ::com::fingerprints::extension::V1_0::IRecalibrationCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

struct FingerprintRecalibration : public IFingerprintRecalibration, public hidl_death_recipient {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintRecalibration follow.
    Return<void> recalibrate(const hidl_vec<uint8_t>& token, const sp<IRecalibrationCallback>& callback) override;
    Return<uint64_t> preRecalibrate() override;
    Return<void> cancel() override;

    static void instantiate(fpc_recalibration_t* device);
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
private:

    static void onStatus(void* context, int32_t code, int32_t imageDecision,
                         int32_t imageQuality, int32_t pnQuality, int32_t progress);
    static void onError(void* context, int32_t code);
    FingerprintRecalibration(fpc_recalibration_t* device);
    virtual ~FingerprintRecalibration();

    static FingerprintRecalibration* sInstance;
    sp<IRecalibrationCallback> mRecalibrationCallback;
    fpc_recalibration_t* mDevice;
};

extern "C" IFingerprintRecalibration* HIDL_FETCH_IFingerprintRecalibration(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTRECALIBRATION_H
