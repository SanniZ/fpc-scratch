/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_authenticator_service.h"
#include "FingerprintAuthenticator.h"

void add_authenticator_service(fpc_authenticator_t* device) {
    android::FingerprintAuthenticator::instantiate(device);
}

namespace android {

FingerprintAuthenticator* FingerprintAuthenticator::sInstance = NULL;

void FingerprintAuthenticator::instantiate(fpc_authenticator_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintAuthenticator(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintAuthenticator::FingerprintAuthenticator(fpc_authenticator_t* device)
        : mVerifyUserCallback(NULL) {
    mDevice = device;
    mVerifyUser = false;
}

FingerprintAuthenticator::~FingerprintAuthenticator() {
}

void FingerprintAuthenticator::binderDied(const wp<IBinder>& who) {
    if (IInterface::asBinder(mVerifyUserCallback) == who) {
        cancel();
    }
}

void FingerprintAuthenticator::onResult(
        void* context, fpc_verify_user_data_t verify_user_data) {
    FingerprintAuthenticator * self =
            static_cast<FingerprintAuthenticator*>(context);
    if (self->mVerifyUserCallback != NULL) {
        self->mVerifyUser = false;
        self->mVerifyUserCallback->onResult(verify_user_data);
    }
}

void FingerprintAuthenticator::onHelp(void* context, int32_t helpCode) {
    FingerprintAuthenticator * self =
            static_cast<FingerprintAuthenticator*>(context);
    if (self->mVerifyUserCallback != NULL) {
        self->mVerifyUser = false;
        self->mVerifyUserCallback->onHelp(helpCode);
    }
}

void FingerprintAuthenticator::verifyUser(
        const sp<IVerifyUserCallback>& callback, const uint8_t* nonce,
        ssize_t nonceLength, const char* dstAppName, ssize_t dstAppNameLength) {
    if (mVerifyUserCallback != NULL
            && IInterface::asBinder(callback)
                    != IInterface::asBinder(mVerifyUserCallback)) {
        IInterface::asBinder(mVerifyUserCallback)->unlinkToDeath(this);
    }
    IInterface::asBinder(callback)->linkToDeath(this);
    mVerifyUserCallback = callback;
    if (mDevice) {
        uint32_t status = mDevice->verify_user(
                mDevice, nonce, nonceLength, dstAppName, dstAppNameLength, this,
                FingerprintAuthenticator::onResult,
                FingerprintAuthenticator::onHelp);
        if (status == 0) {
            mVerifyUser = true;
        }
    }
}

void FingerprintAuthenticator::isUserValid(int64_t userId, bool* result) {
    if (mDevice) {
        mDevice->is_user_valid(mDevice, userId, result);
    }
}

void FingerprintAuthenticator::cancel() {
    if (mVerifyUserCallback != NULL) {
        IInterface::asBinder(mVerifyUserCallback)->unlinkToDeath(this);
    }
    mVerifyUserCallback = NULL;
    if (mDevice && mVerifyUser) {
        mDevice->cancel(mDevice);
    }
}

}
