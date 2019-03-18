/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "FingerprintAuthenticator.h"
#include <utils/Log.h>

void add_authenticator_service(fpc_authenticator_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintAuthenticator::instantiate(
            device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintAuthenticator* FingerprintAuthenticator::sInstance = NULL;

void FingerprintAuthenticator::instantiate(fpc_authenticator_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintAuthenticator(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintAuthenticator");
        }
    }
}

FingerprintAuthenticator::FingerprintAuthenticator(fpc_authenticator_t* device)
        : mVerifyUserCallback(NULL),
          mDevice(device),
          mVerifyUser(device) {
}

void FingerprintAuthenticator::onResult(void* context, fpc_verify_user_data_t verify_user_data) {
    FingerprintAuthenticator * self = static_cast<FingerprintAuthenticator*>(context);
    if (self->mVerifyUserCallback != NULL) {
        self->mVerifyUser = false;
        hidl_vec<uint8_t> encapsulatedResult;
        encapsulatedResult.setToExternal(verify_user_data.result_blob,
                                         verify_user_data.size_result_blob);
        if (!self->mVerifyUserCallback->onResult(verify_user_data.result,
                                                 verify_user_data.user_id,
                                                 verify_user_data.entity_id,
                                                 encapsulatedResult).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintAuthenticator::onHelp(void* context, int32_t helpCode) {
    FingerprintAuthenticator * self =
            static_cast<FingerprintAuthenticator*>(context);
    if (self->mVerifyUserCallback != NULL) {
        self->mVerifyUser = false;
        if (!self->mVerifyUserCallback->onHelp(helpCode).isOk()) {
            ALOGE("%s callback failed", __func__);
        }
    }
}

void FingerprintAuthenticator::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    (void)cookie;
    if (who == mVerifyUserCallback) {
        cancel();
    }
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintAuthenticator follow.
Return<uint32_t> FingerprintAuthenticator::verifyUser(const sp<IVerifyUserCallback>& callback,
                                                      const hidl_vec<uint8_t>& nonce,
                                                      const hidl_vec<uint8_t>& dstAppName) {

    if (mVerifyUserCallback != NULL) {
        mVerifyUserCallback->unlinkToDeath(this);
    }
    mVerifyUserCallback = callback;

    if (mVerifyUserCallback != NULL) {
        mVerifyUserCallback->linkToDeath(this, 0);
    }

    uint32_t status = -1;
    if (mDevice) {
        status = mDevice->verify_user(mDevice,
                                      nonce.data(),
                                      nonce.size(),
                                      reinterpret_cast<const char *>(dstAppName.data()),
                                      dstAppName.size(), this,
                                      FingerprintAuthenticator::onResult,
                                      FingerprintAuthenticator::onHelp);
        if (status == 0) {
            mVerifyUser = true;
        }
    }
    return status;
}

Return<bool> FingerprintAuthenticator::isUserValid(uint64_t userId) {
    bool is_user_valid = false;
    if (mDevice) {
        mDevice->is_user_valid(mDevice, userId, &is_user_valid);
    }
    return is_user_valid;
}

Return<void> FingerprintAuthenticator::cancel() {

    if (mVerifyUserCallback != NULL) {
        mVerifyUserCallback->unlinkToDeath(this);
        mVerifyUserCallback = NULL;
    }

    if (mDevice && mVerifyUser) {
        mDevice->cancel(mDevice);
    }
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
