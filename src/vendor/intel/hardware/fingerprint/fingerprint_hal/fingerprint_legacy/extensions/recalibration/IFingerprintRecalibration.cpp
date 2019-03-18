/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>
#include <utils/Log.h>
#include "IFingerprintRecalibration.h"

/* When a interface operation uses a parcel class to pass data the first 4 bytes written/read is
 * header information.
 * Use reply->writeInt32(HAS_PARCEL_DATA) to write a valid parcel header then proceed to
 * write the actual data.
 * Use reply->writeInt32(NO_PARCEL_DATA) to write null as reply parcel.
 * Use int parcel_header = data.readInt32() to read the header when receiving a parcel before
 * proceeding to read the actual data.
 */
#define NO_PARCEL_DATA 0
#define HAS_PARCEL_DATA 1

namespace android {

static const String16 FINGERPRINT_EXTENSION_RECALIBRATION(
        "com.fingerprints.extension.RECALIBRATION");

/***** RecalibrationCallback *****/

class BpRecalibrationCallback : public BpInterface<IRecalibrationCallback> {
     public:
        BpRecalibrationCallback(const sp<IBinder>& impl)
                : BpInterface<IRecalibrationCallback>(impl) {
        }

        virtual void onStatus(int32_t code, int32_t imageDecision,
                int32_t imageQuality, int32_t pnQuality, int32_t progress) {
            Parcel data, reply;
            data.writeInterfaceToken(
                                IRecalibrationCallback::getInterfaceDescriptor());

            data.writeInt32(code);
            data.writeInt32(imageDecision);
            data.writeInt32(imageQuality);
            data.writeInt32(pnQuality);
            data.writeInt32(progress);

            status_t status = remote()->transact(ON_STATUS, data, &reply,
                                                 IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
               ALOGD("onStatus() could not contact remote: %d\n", status);
            }
        }

        virtual void onError(int32_t error) {
            Parcel data, reply;
            data.writeInterfaceToken(
                                IRecalibrationCallback::getInterfaceDescriptor());

            data.writeInt32(error);

            status_t status = remote()->transact(ON_ERROR, data, &reply,
                                                 IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
              ALOGD("onError() could not contact remote: %d\n", status);
            }
        }
};

IMPLEMENT_META_INTERFACE(RecalibrationCallback,
        "com.fingerprints.extension.recalibration.IRecalibrationCallback");


/***** IFingerprintCalibration *****/

const android::String16 IFingerprintRecalibration::descriptor(
        "com.fingerprints.extension.recalibration.IFingerprintRecalibration");

const android::String16& IFingerprintRecalibration::getInterfaceDescriptor() const {
    return IFingerprintRecalibration::descriptor;
}

status_t BnFingerprintRecalibration::onTransact(uint32_t code, const Parcel& data,
                                              Parcel* reply, uint32_t flags) {
    switch (code) {
        case RECALIBRATE: {
            CHECK_INTERFACE(IFingerprintRecalibration, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_RECALIBRATION)) {
                return PERMISSION_DENIED;
            }

            ssize_t tokenLength = data.readInt32();

            const uint8_t* token = static_cast<const uint8_t *>(data.readInplace(
                    tokenLength));

            sp < IRecalibrationCallback > callback = interface_cast
                                < IRecalibrationCallback > (data.readStrongBinder());

            reply->writeNoException();

            recalibrate(token, tokenLength, callback);

            return NO_ERROR;
        }
        case PRERECALIBRATE: {
            CHECK_INTERFACE(IFingerprintRecalibration, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_RECALIBRATION)) {
                return PERMISSION_DENIED;
            }

            reply->writeNoException();

            uint64_t challenge;

            preRecalibrate(&challenge);

            reply->writeInt64(challenge);

            return NO_ERROR;
        }
        case CANCEL: {
            CHECK_INTERFACE(IFingerprintRecalibration, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_RECALIBRATION)) {
                return PERMISSION_DENIED;
            }
            reply->writeNoException();

            cancel();

            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintRecalibration::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
