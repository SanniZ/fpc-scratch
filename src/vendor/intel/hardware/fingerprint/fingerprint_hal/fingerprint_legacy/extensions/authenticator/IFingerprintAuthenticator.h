/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_AUTHENTICATOR_H_
#define IFINGERPRINT_AUTHENTICATOR_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_authenticator.h"

namespace android {

class IVerifyUserCallback : public IInterface {
     public:
        // must be kept in sync with IVerifyUserCallback.aidl
        enum {
            ON_RESULT = IBinder::FIRST_CALL_TRANSACTION,
            ON_HELP,
        };

        virtual void onResult(fpc_verify_user_data_t verify_user_data) = 0;
        virtual void onHelp(int32_t helpCode) = 0;

        DECLARE_META_INTERFACE (VerifyUserCallback);
};

class IFingerprintAuthenticator : public IInterface {
     public:
        // must be kept in sync with IFingerprintAuthenticator.aidl
        enum {
            VERIFY_USER = IBinder::FIRST_CALL_TRANSACTION,
            IS_USER_VALID,
            CANCEL,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void verifyUser(const sp<IVerifyUserCallback>& callback,
                                const uint8_t* nonce, ssize_t nonceLength,
                                const char* dstAppName,
                                ssize_t dstAppNameLength) = 0;
        virtual void isUserValid(int64_t userId, bool* result) = 0;
        virtual void cancel() = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintAuthenticator : public BnInterface<IFingerprintAuthenticator> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);
     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_AUTHENTICATOR_H_

