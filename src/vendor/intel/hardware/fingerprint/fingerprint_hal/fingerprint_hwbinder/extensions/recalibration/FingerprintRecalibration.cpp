#include "FingerprintRecalibration.h"
#include "fpc_hal_ext_recalibration_service.h"
#include <utils/Log.h>

void add_recalibration_service(fpc_recalibration_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintRecalibration::instantiate(
            device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintRecalibration* FingerprintRecalibration::sInstance = NULL;

void FingerprintRecalibration::instantiate(fpc_recalibration_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintRecalibration(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintRecalibration");
        }
    }
}

FingerprintRecalibration::FingerprintRecalibration(fpc_recalibration_t* device)
        : mRecalibrationCallback(NULL),
          mDevice(device) {
}

FingerprintRecalibration::~FingerprintRecalibration() {
}

void FingerprintRecalibration::onStatus(void* context,
                                        int32_t code,
                                        int32_t imageDecision,
                                        int32_t imageQuality,
                                        int32_t pnQuality,
                                        int32_t progress) {

    FingerprintRecalibration * self = static_cast<FingerprintRecalibration*>(context);

    if (self->mRecalibrationCallback != NULL) {
        if (!self->mRecalibrationCallback->onStatus(code,
                                                    imageDecision,
                                                    imageQuality,
                                                    pnQuality,
                                                    progress).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintRecalibration::onError(void* context, int32_t code) {
    FingerprintRecalibration * self = static_cast<FingerprintRecalibration*>(context);

    if (self->mRecalibrationCallback != NULL) {
        if (!self->mRecalibrationCallback->onError(code).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintRecalibration::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    (void)cookie;
    if (who == mRecalibrationCallback) {
        cancel();
    }
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintRecalibration follow.
Return<void> FingerprintRecalibration::recalibrate(const hidl_vec<uint8_t>& token,
                                                   const sp<IRecalibrationCallback>& callback) {
    if (mRecalibrationCallback != NULL) {
        mRecalibrationCallback->unlinkToDeath(this);
    }
    mRecalibrationCallback = callback;
    if (mRecalibrationCallback != NULL) {
        mRecalibrationCallback->linkToDeath(this, 0);
    }

    if (mDevice) {
        mDevice->set_recalibration_cb(mDevice,
                                      FingerprintRecalibration::onStatus,
                                      FingerprintRecalibration::onError,
                                      this);
        mDevice->recalibrate_pn(mDevice, token.data(), token.size());
    }
    return Void();
}

Return<uint64_t> FingerprintRecalibration::preRecalibrate() {
    uint64_t challenge = 0;
    if (mDevice) {
        mDevice->pre_recalibrate_pn(mDevice, &challenge);
    }
    return challenge;
}

Return<void> FingerprintRecalibration::cancel() {
    if (mDevice) {
        mDevice->cancel_recalibration(mDevice);
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
