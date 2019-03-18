/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "IFingerprintSenseTouch.h"

#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <utils/String16.h>
#include <utils/Log.h>


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

static const String16 FINGERPRINT_EXTENSION_SENSE_TOUCH(
        "com.fingerprints.extension.SENSE_TOUCH");

/***** IFingerprintSensorForce *****/

const android::String16 IFingerprintSenseTouch::descriptor(
        "com.fingerprints.extension.sensetouch.IFingerprintSenseTouch");

const android::String16& IFingerprintSenseTouch::getInterfaceDescriptor() const {
    return IFingerprintSenseTouch::descriptor;
}

status_t BnFingerprintSenseTouch::onTransact(uint32_t code, const Parcel& data,
                                              Parcel* reply, uint32_t flags) {
    switch (code) {
        case GET_FORCE: {
            CHECK_INTERFACE(IFingerprintSenseTouch, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSE_TOUCH)) {
                return PERMISSION_DENIED;
            }

            reply->writeNoException();

            uint8_t force;

            getForce(&force);

            reply->writeInt32(force);

            return NO_ERROR;
        }
        case IS_SUPPORTED: {
            CHECK_INTERFACE(IFingerprintSenseTouch, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSE_TOUCH)) {
                return PERMISSION_DENIED;
            }

            reply->writeNoException();

            uint8_t result;

            isSupported(&result);

            reply->writeInt32(result ? 1 : 0);

            return NO_ERROR;
        }
        case FINISH_CALIBRATION: {
            ALOGD("Received binder transaction FINISH_CALIBRATION(%u)", code);
            CHECK_INTERFACE(IFingerprintSenseTouch, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSE_TOUCH)) {
                return PERMISSION_DENIED;
            }

            reply->writeNoException();

            uint8_t ground = data.readInt32();
            uint8_t threshold = data.readInt32();

            int status = storeCalibrationData(ground, threshold);
            reply->writeInt32(status == 0 ? 1 : 0);

            return NO_ERROR;
        }
        case SET_AUTH_MODE: {
            CHECK_INTERFACE(IFingerprintSenseTouch, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSE_TOUCH)) {
                return PERMISSION_DENIED;
            }

            reply->writeNoException();

            SenseTouchAuthenticationMode mode = (SenseTouchAuthenticationMode) data.readInt32();
            uint32_t timeoutMs = data.readInt32();

            int status = setAuthMode(mode, timeoutMs);

            reply->writeInt32(status == 0 ? 1 : 0);

            return NO_ERROR;
        }
        case READ_CONFIG: {
            ALOGD("Received binder transaction READ_CONFIG(%u)", code);
            CHECK_INTERFACE(IFingerprintSenseTouch, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_SENSE_TOUCH)) {
                return PERMISSION_DENIED;
            }

            const fpc_sense_touch_config_t* config = NULL;
            int status = readConfig(&config);

            reply->writeNoException();
            // This is the return code
            reply->writeInt32(status == 0 ? 1 : 0);

            if(!status && config) {
                reply->writeInt32(HAS_PARCEL_DATA);
                // This is the parcel data
                reply->writeInt32(config->version);
                reply->writeInt32(config->ground);
                reply->writeInt32(config->trigger_threshold);
                reply->writeInt32(config->untrigger_threshold);
                reply->writeInt32(config->auth_enable_down_force);
                reply->writeInt32(config->auth_enable_up_force);
                reply->writeInt32(config->auth_button_timeout_ms);
            } else {
                ALOGE("Unable to write parcel information for READ_CONFIG(%d), operation failed "
                      "with code: %d.", READ_CONFIG, status);
                reply->writeInt32(NO_PARCEL_DATA);
            }

            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintSenseTouch::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
