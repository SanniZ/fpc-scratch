/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_recalibration_service.h"
#include "FingerprintRecalibration.h"
#include <android/log.h>

void add_recalibration_service(fpc_recalibration_t* device) {
    android::FingerprintRecalibration::instantiate(device);
}

namespace android {

FingerprintRecalibration* FingerprintRecalibration::sInstance = NULL;

void FingerprintRecalibration::instantiate(fpc_recalibration_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintRecalibration(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintRecalibration::FingerprintRecalibration(fpc_recalibration_t* device)
        : mRecalibrationCallback(NULL) {
    mDevice = device;
}

FingerprintRecalibration::~FingerprintRecalibration() {
}

void FingerprintRecalibration::binderDied(const wp<IBinder>& who) {
    if (IInterface::asBinder(mRecalibrationCallback) == who) {
        mRecalibrationCallback = NULL;
    }
}

void FingerprintRecalibration::onStatus(void* context, int32_t code, int32_t imageDecision,
        int32_t imageQuality, int32_t pnQuality, int32_t progress) {

    FingerprintRecalibration * self =
            static_cast<FingerprintRecalibration*>(context);

    if (self->mRecalibrationCallback != NULL) {
        self->mRecalibrationCallback->onStatus(code, imageDecision, imageQuality, pnQuality, progress);
    }
}

void FingerprintRecalibration::onError(void* context, int32_t code) {
    FingerprintRecalibration * self =
            static_cast<FingerprintRecalibration*>(context);

    if (self->mRecalibrationCallback != NULL) {
        self->mRecalibrationCallback->onError(code);
    }
}

void FingerprintRecalibration::recalibrate(const uint8_t* token, ssize_t tokenLength,
        const sp<IRecalibrationCallback>& callback) {

    if (mRecalibrationCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mRecalibrationCallback)) {
        IInterface::asBinder(mRecalibrationCallback)->unlinkToDeath(this);
    }

    IInterface::asBinder(callback)->linkToDeath(this);

    mRecalibrationCallback = callback;
    if (mDevice) {
        mDevice->set_recalibration_cb(mDevice, FingerprintRecalibration::onStatus,
                FingerprintRecalibration::onError, this);
        mDevice->recalibrate_pn(mDevice, token, tokenLength);
    }
}

void FingerprintRecalibration::preRecalibrate(uint64_t* challenge) {
    if (mDevice) {
        mDevice->pre_recalibrate_pn(mDevice, challenge);
    }
}

void FingerprintRecalibration::cancel() {
    if (mDevice) {
        mDevice->cancel_recalibration(mDevice);
    }
}

}
