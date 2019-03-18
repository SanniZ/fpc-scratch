/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_engineering_service.h"
#include "FingerprintEngineering.h"

void add_engineering_service(fpc_engineering_t* device) {
    android::FingerprintEngineering::instantiate(device);
}

namespace android {

FingerprintEngineering* FingerprintEngineering::sInstance = NULL;

void FingerprintEngineering::instantiate(fpc_engineering_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintEngineering(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintEngineering::FingerprintEngineering(fpc_engineering_t* device)
        : mImageSubscriptionCallback(NULL),
          mImageInjectionCallback(NULL),
          mCaptureCallback(NULL) {
    mDevice = device;
}

FingerprintEngineering::~FingerprintEngineering() {
}

void FingerprintEngineering::binderDied(const wp<IBinder>& who) {
    if (IInterface::asBinder(mImageSubscriptionCallback) == who) {
        stopImageSubscription();
    } else if (IInterface::asBinder(mImageInjectionCallback) == who) {
        stopImageInjection();
    } else if (IInterface::asBinder(mCaptureCallback) == who) {
        cancelCapture();
    }
}

void FingerprintEngineering::onImage(void* context,
                                     fpc_capture_data_t* capture_data) {
    FingerprintEngineering * self =
            static_cast<FingerprintEngineering*>(context);
    if (self->mImageSubscriptionCallback != NULL) {
        self->mImageSubscriptionCallback->onImage(capture_data);
    }
}

int FingerprintEngineering::onInject(void* context,
                                     fpc_hal_img_data_t* img_data) {
    FingerprintEngineering * self =
            static_cast<FingerprintEngineering*>(context);
    if (self->mImageInjectionCallback != NULL) {
        self->mImageInjectionCallback->onInject(img_data);
    }
    return 0;
}

void FingerprintEngineering::onCancel(void* context) {
    FingerprintEngineering * self =
            static_cast<FingerprintEngineering*>(context);
    if (self->mImageInjectionCallback != NULL) {
        self->mImageInjectionCallback->onCancel();
    }
}

void FingerprintEngineering::getSensorSize(uint8_t* width, uint8_t* height) {
    if (mDevice) {
        mDevice->get_sensor_size(mDevice, width, height);
    }
}

void FingerprintEngineering::startImageSubscription(
        const sp<IImageSubscriptionCallback>& callback) {
    if (mImageSubscriptionCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mImageSubscriptionCallback)) {
        IInterface::asBinder(mImageSubscriptionCallback)->unlinkToDeath(this);
    }
    IInterface::asBinder(callback)->linkToDeath(this);
    mImageSubscriptionCallback = callback;
    if (mDevice) {
        mDevice->set_img_subscr_cb(mDevice, FingerprintEngineering::onImage,
                                   this);
    }
}

void FingerprintEngineering::stopImageSubscription() {
    if (mImageSubscriptionCallback != NULL) {
        IInterface::asBinder(mImageSubscriptionCallback)->unlinkToDeath(this);
    }
    mImageSubscriptionCallback = NULL;
    if (mDevice) {
        mDevice->set_img_subscr_cb(mDevice, NULL, NULL);
    }
}

void FingerprintEngineering::startImageInjection(
        const sp<IImageInjectionCallback>& callback) {
    if (mImageInjectionCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mImageInjectionCallback)) {
        IInterface::asBinder(mImageInjectionCallback)->unlinkToDeath(this);
    }
    IInterface::asBinder(callback)->linkToDeath(this);
    mImageInjectionCallback = callback;
    if (mDevice) {
        mDevice->set_img_inj_cb(mDevice, FingerprintEngineering::onInject,
                                FingerprintEngineering::onCancel, this);
    }
}

void FingerprintEngineering::stopImageInjection() {
    if (mImageInjectionCallback != NULL) {
        IInterface::asBinder(mImageInjectionCallback)->unlinkToDeath(this);
    }
    mImageInjectionCallback = NULL;
    if (mDevice) {
        mDevice->set_img_inj_cb(mDevice, NULL, NULL, NULL);
    }
}

void FingerprintEngineering::onCapture(void* context,
                                      fpc_capture_data_t *capture_data) {
    FingerprintEngineering * self =
            static_cast<FingerprintEngineering*>(context);
    sp<ICaptureCallback> captureCallback = self->mCaptureCallback;
    if (!capture_data->samples_remaining) {
        self->mCaptureCallback = NULL;
    }
    if (captureCallback != NULL) {
        if (capture_data->samples_remaining == 0) {
            self->isCapture = false;
        }

        captureCallback->onCapture(capture_data);
    }
}

void FingerprintEngineering::setEnrollToken(const uint8_t* token, ssize_t tokenLength) {
    if (mDevice) {
        mDevice->set_enroll_token(mDevice, token, tokenLength);
    }
}

void FingerprintEngineering::getEnrollChallenge(uint64_t* challenge) {
    if (mDevice) {
        mDevice->get_enroll_challenge(mDevice, challenge);
    }
}

void FingerprintEngineering::startCapture(const sp<ICaptureCallback>& callback,
                                          fpc_capture_mode_t mode) {
    if (mCaptureCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mCaptureCallback)) {
        IInterface::asBinder(callback)->linkToDeath(this);
    }

    mCaptureCallback = callback;

    if (mDevice) {
        isCapture = true;
        mDevice->start_capture(mDevice, FingerprintEngineering::onCapture, mode, this);
    }
}

void FingerprintEngineering::cancelCapture() {
    if (mCaptureCallback != NULL) {
        IInterface::asBinder(mCaptureCallback)->unlinkToDeath(this);
    }

    if (mDevice && isCapture) {
        mDevice->cancel_capture(mDevice);
        isCapture = false;
    }
}
}
