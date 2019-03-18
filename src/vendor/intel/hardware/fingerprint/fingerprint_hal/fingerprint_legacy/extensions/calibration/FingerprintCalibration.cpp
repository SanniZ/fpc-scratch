/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_calibration_service.h"
#include "FingerprintCalibration.h"
#include <android/log.h>

void add_calibration_service(fpc_calibration_t* device) {
    android::FingerprintCalibration::instantiate(device);
}

namespace android {

FingerprintCalibration* FingerprintCalibration::sInstance = NULL;

void FingerprintCalibration::instantiate(fpc_calibration_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintCalibration(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintCalibration::FingerprintCalibration(fpc_calibration_t* device)
        : mCalibrationCallback(NULL) {
    mDevice = device;
}

FingerprintCalibration::~FingerprintCalibration() {
}

void FingerprintCalibration::binderDied(const wp<IBinder>& who) {
    (void) who;
}

int FingerprintCalibration::onStatus(void* context, uint8_t code) {
    int status = 0;
    FingerprintCalibration * self =
            static_cast<FingerprintCalibration*>(context);

    if (self->mCalibrationCallback != NULL) {
        self->mCalibrationCallback->onStatus(code);
    } else {
        status = 1;
    }

    return status;
}

int FingerprintCalibration::onError(void* context, int8_t code) {
    int status = 0;
    FingerprintCalibration * self =
            static_cast<FingerprintCalibration*>(context);

    if (self->mCalibrationCallback != NULL) {
        self->mCalibrationCallback->onError(code);
    } else {
        status = 1;
    }

    return status;
}

void FingerprintCalibration::calibrate(
        const sp<ICalibrationCallback>& callback) {

    if (mCalibrationCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mCalibrationCallback)) {
        IInterface::asBinder(mCalibrationCallback)->unlinkToDeath(this);
    }

    IInterface::asBinder(callback)->linkToDeath(this);

    mCalibrationCallback = callback;

    if (mDevice) {
        mDevice->set_calibration_cb(mDevice, FingerprintCalibration::onStatus,
                FingerprintCalibration::onError, this);

        mDevice->calibrate_pn(mDevice);
    }
}

}
