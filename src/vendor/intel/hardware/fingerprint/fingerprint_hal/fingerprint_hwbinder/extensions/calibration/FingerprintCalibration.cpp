/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "fpc_hal_ext_calibration_service.h"
#include "FingerprintCalibration.h"
#include <utils/Log.h>

void add_calibration_service(fpc_calibration_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintCalibration::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintCalibration* FingerprintCalibration::sInstance = NULL;

void FingerprintCalibration::instantiate(fpc_calibration_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintCalibration(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintCalibration");
        }
    }
}

FingerprintCalibration::FingerprintCalibration(fpc_calibration_t* device)
        : mCalibrationCallback(NULL),
          mDevice(device) {
}

int FingerprintCalibration::onStatus(void* context, uint8_t code) {
    int status = 0;
    FingerprintCalibration * self =
            static_cast<FingerprintCalibration*>(context);

    if (self->mCalibrationCallback != NULL) {
        if (!self->mCalibrationCallback->onStatus(code).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    } else {
        status = 1;
    }

    return status;
}

int FingerprintCalibration::onError(void* context, int8_t code) {
    int status = 0;
    FingerprintCalibration * self = static_cast<FingerprintCalibration*>(context);

    if (self->mCalibrationCallback != NULL) {
        if (!self->mCalibrationCallback->onError(code).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    } else {
        status = 1;
    }

    return status;
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintCalibration follow.
Return<void> FingerprintCalibration::calibrate(const sp<ICalibrationCallback>& callback) {
    mCalibrationCallback = callback;

    if (mDevice) {
        mDevice->set_calibration_cb(mDevice, FingerprintCalibration::onStatus,
                                    FingerprintCalibration::onError, this);

        mDevice->calibrate_pn(mDevice);
    }
    return Void();
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
