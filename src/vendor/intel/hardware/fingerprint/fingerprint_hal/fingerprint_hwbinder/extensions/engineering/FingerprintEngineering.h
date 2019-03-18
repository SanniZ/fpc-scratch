#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H

#include <com/fingerprints/extension/1.0/IFingerprintEngineering.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "fpc_hal_ext_engineering_service.h"
#include "fpc_hal_ext_engineering.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::com::fingerprints::extension::V1_0::IFingerprintEngineering;
using ::com::fingerprints::extension::V1_0::IImageCaptureCallback;
using ::com::fingerprints::extension::V1_0::IImageInjectionCallback;
using ::com::fingerprints::extension::V1_0::SensorSize;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

struct FingerprintEngineering : public IFingerprintEngineering, public hidl_death_recipient {
    // Methods from ::com::fingerprints::extension::V1_0::IFingerprintEngineering follow.
    Return<void> getSensorSize(getSensorSize_cb _hidl_cb) override;
    Return<void> startImageSubscription(const sp<IImageCaptureCallback>& callback) override;
    Return<void> stopImageSubscription() override;
    Return<void> startImageInjection(const sp<IImageInjectionCallback>& callback) override;
    Return<void> stopImageInjection() override;
    Return<void> startCapture(const sp<IImageCaptureCallback>& callback, uint32_t mode) override;
    Return<void> cancelCapture() override;
    Return<void> setEnrollToken(const hidl_vec<uint8_t>& token) override;
    Return<uint64_t> getEnrollChallenge() override;

    static void instantiate(fpc_engineering_t* device);
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

private:
    static void onInjectCallback(const hidl_vec<uint8_t>& imageData);
    static void onImage(void* context, fpc_capture_data_t* captureData);
    static int onInject(void* context, fpc_hal_img_data_t* imageData);
    static void onCancel(void* context);
    static void onCapture(void* context, fpc_capture_data_t* captureData);
    FingerprintEngineering(fpc_engineering_t* device);
    virtual ~FingerprintEngineering();

    static FingerprintEngineering* sInstance;
    sp<IImageCaptureCallback> mImageSubscriptionCallback;
    sp<IImageInjectionCallback> mImageInjectionCallback;
    sp<IImageCaptureCallback> mCaptureCallback;
    fpc_engineering_t* mDevice;
    bool mIsCapture;
    static fpc_hal_img_data_t *sImageInjectData;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_FINGERPRINTENGINEERING_H
