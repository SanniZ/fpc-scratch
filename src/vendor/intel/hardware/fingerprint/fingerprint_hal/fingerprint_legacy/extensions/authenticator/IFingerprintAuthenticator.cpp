/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>
#include "IFingerprintAuthenticator.h"

namespace android {

static const String16 FINGERPRINT_EXTENSION_AUTHENTICATOR(
        "com.fingerprints.extension.AUTHENTICATOR");

/***** IVerifyUserCallback *****/

class BpVerifyUserCallback : public BpInterface<IVerifyUserCallback> {
     public:
        BpVerifyUserCallback(const sp<IBinder>& impl)
                : BpInterface<IVerifyUserCallback>(impl) {
        }

        virtual void onResult(fpc_verify_user_data_t verify_user_data) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    IVerifyUserCallback::getInterfaceDescriptor());
            data.writeInt32(verify_user_data.result);
            data.writeInt64(verify_user_data.user_id);
            data.writeInt64(verify_user_data.entity_id);
            data.writeByteArray(verify_user_data.size_result_blob,
                                verify_user_data.result_blob);
            remote()->transact(ON_RESULT, data, &reply, IBinder::FLAG_ONEWAY);
        }

        virtual void onHelp(int32_t result) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    IVerifyUserCallback::getInterfaceDescriptor());
            data.writeInt32(result);
            remote()->transact(ON_HELP, data, &reply, IBinder::FLAG_ONEWAY);
        }
};

IMPLEMENT_META_INTERFACE(VerifyUserCallback,
        "com.fingerprints.extension.authenticator.IVerifyUserCallback");

/***** IFingerprintAuthenticator *****/

const android::String16 IFingerprintAuthenticator::descriptor(
        "com.fingerprints.extension.authenticator.IFingerprintAuthenticator");

const android::String16& IFingerprintAuthenticator::getInterfaceDescriptor() const {
    return IFingerprintAuthenticator::descriptor;
}

status_t BnFingerprintAuthenticator::onTransact(uint32_t code,
                                                const Parcel& data,
                                                Parcel* reply, uint32_t flags) {
    switch (code) {
        case VERIFY_USER: {
            CHECK_INTERFACE(IFingerprintAuthenticator, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_AUTHENTICATOR)) {
                return PERMISSION_DENIED;
            }
            sp < IVerifyUserCallback > callback = interface_cast
                    < IVerifyUserCallback > (data.readStrongBinder());
            const ssize_t nonceLength = data.readInt32();
            const uint8_t* nonce =
                    static_cast<const uint8_t *>(data.readInplace(nonceLength));
            const ssize_t dstAppNameLength = data.readInt32();
            const char* dstAppName = reinterpret_cast<const char*>(data
                    .readInplace(dstAppNameLength));
            verifyUser(callback, nonce, nonceLength, dstAppName,
                       dstAppNameLength);
            reply->writeNoException();
            return NO_ERROR;
        }
        case IS_USER_VALID: {
            CHECK_INTERFACE(IFingerprintAuthenticator, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_AUTHENTICATOR)) {
                return PERMISSION_DENIED;
            }
            int64_t userId = data.readInt64();
            bool result;
            isUserValid(userId, &result);
            reply->writeNoException();
            reply->writeInt32(result ? 1 : 0);
            return NO_ERROR;
        }
        case CANCEL: {
            CHECK_INTERFACE(IFingerprintAuthenticator, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_AUTHENTICATOR)) {
                return PERMISSION_DENIED;
            }
            cancel();
            reply->writeNoException();
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintAuthenticator::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
