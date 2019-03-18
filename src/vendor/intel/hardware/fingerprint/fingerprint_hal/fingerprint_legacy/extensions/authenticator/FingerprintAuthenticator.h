/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_AUTHENTICATOR_H_
#define FINGERPRINT_AUTHENTICATOR_H_

#include "IFingerprintAuthenticator.h"
#include "fpc_hal_ext_authenticator.h"

namespace android {

class FingerprintAuthenticator : public BnFingerprintAuthenticator,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_authenticator_t* device);
        static void onResult(void* context,
                             fpc_verify_user_data_t verify_user_data);
        static void onHelp(void* context, int32_t helpCode);

        // These reflect binder methods.
        virtual void verifyUser(const sp<IVerifyUserCallback>& callback,
                                const uint8_t* nonce, ssize_t nonceLength,
                                const char* dstAppName,
                                ssize_t dstAppNameLength);
        virtual void isUserValid(int64_t userId, bool* result);
        virtual void cancel();

     private:
        FingerprintAuthenticator(fpc_authenticator_t* device);
        virtual ~FingerprintAuthenticator();
        void binderDied(const wp<IBinder>& who);

        static FingerprintAuthenticator* sInstance;
        sp<IVerifyUserCallback> mVerifyUserCallback;
        fpc_authenticator_t* mDevice;
        bool mVerifyUser;
};

}  // namespace android

#endif // FINGERPRINT_AUTHENTICATOR_H_
