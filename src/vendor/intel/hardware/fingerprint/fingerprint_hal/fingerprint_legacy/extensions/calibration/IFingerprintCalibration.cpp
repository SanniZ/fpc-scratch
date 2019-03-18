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
#include <utils/Log.h>
#include "IFingerprintCalibration.h"

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

static const String16 FINGERPRINT_EXTENSION_CALIBRATION(
        "com.fingerprints.extension.CALIBRATION");

/***** CalibrationCallback *****/

class BpCalibrationCallback : public BpInterface<ICalibrationCallback> {
     public:
        BpCalibrationCallback(const sp<IBinder>& impl)
                : BpInterface<ICalibrationCallback>(impl) {
        }

        virtual void onStatus(uint8_t code) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    ICalibrationCallback::getInterfaceDescriptor());

            data.writeInt32(code);

            status_t status = remote()->transact(ON_CALIBRATION_STATUS, data,
                                                 &reply, IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
                ALOGD("onStatus() could not contact remote: %d\n", status);
            }
        }

        virtual void onError(int8_t error) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    ICalibrationCallback::getInterfaceDescriptor());

            data.writeInt32(error);

            status_t status = remote()->transact(ON_CALIBRATION_ERROR, data,
                                                 &reply, IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
                ALOGD("onError() could not contact remote: %d\n", status);
            }
        }
};

IMPLEMENT_META_INTERFACE(CalibrationCallback,
        "com.fingerprints.extension.calibration.ICalibrationCallback");

/***** IFingerprintCalibration *****/

const android::String16 IFingerprintCalibration::descriptor(
        "com.fingerprints.extension.calibration.IFingerprintCalibration");

const android::String16& IFingerprintCalibration::getInterfaceDescriptor() const {
    return IFingerprintCalibration::descriptor;
}

status_t BnFingerprintCalibration::onTransact(uint32_t code, const Parcel& data,
                                              Parcel* reply, uint32_t flags) {
    switch (code) {
        case CALIBRATE: {
            CHECK_INTERFACE(IFingerprintCalibration, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_CALIBRATION)) {
                return PERMISSION_DENIED;
            }

            sp < ICalibrationCallback > callback = interface_cast
                    < ICalibrationCallback > (data.readStrongBinder());

            reply->writeNoException();

            calibrate (callback);

            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintCalibration::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
