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
#include "IFingerprintNavigation.h"

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

static const String16 FINGERPRINT_EXTENSION_NAVIGATION(
        "com.fingerprints.extension.NAVIGATION");

/***** IFingerprintNavigation *****/

const android::String16 IFingerprintNavigation::descriptor(
        "com.fingerprints.extension.navigation.IFingerprintNavigation");

const android::String16&
IFingerprintNavigation::getInterfaceDescriptor() const {
    return IFingerprintNavigation::descriptor;
}

status_t BnFingerprintNavigation::onTransact(uint32_t code, const Parcel& data,
                                             Parcel* reply, uint32_t flags) {
    switch (code) {
        case SET_NAVIGATION: {
            CHECK_INTERFACE(IFingerprintNavigation, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_NAVIGATION)) {
                return PERMISSION_DENIED;
            }
            bool enabled = data.readInt32() != 0;
            setNavigation(enabled);
            reply->writeNoException();
            return NO_ERROR;
        }
        case GET_NAVIGATION_CONFIG: {
            CHECK_INTERFACE(IFingerprintNavigation, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_NAVIGATION)) {
                return PERMISSION_DENIED;
            }
            fpc_nav_config_t config;
            getNavigationConfig(&config);
            reply->writeNoException();
            reply->writeInt32(HAS_PARCEL_DATA);
            reply->writeInt32(config.single_click_min_time_threshold);
            reply->writeInt32(config.hold_click_time_threshold);
            reply->writeInt32(config.double_click_time_interval);
            reply->writeInt32(config.fast_move_tolerance);
            reply->writeInt32(config.slow_swipe_up_threshold);
            reply->writeInt32(config.slow_swipe_down_threshold);
            reply->writeInt32(config.slow_swipe_left_threshold);
            reply->writeInt32(config.slow_swipe_right_threshold);
            reply->writeInt32(config.fast_swipe_up_threshold);
            reply->writeInt32(config.fast_swipe_down_threshold);
            reply->writeInt32(config.fast_swipe_left_threshold);
            reply->writeInt32(config.fast_swipe_right_threshold);
            return NO_ERROR;
        }
        case SET_NAVIGATION_CONFIG: {
            CHECK_INTERFACE(IFingerprintNavigation, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_NAVIGATION)) {
                return PERMISSION_DENIED;
            }
            fpc_nav_config_t config;
            int parcel_header = data.readInt32();
            if (parcel_header == HAS_PARCEL_DATA) {
                config.single_click_min_time_threshold = data.readInt32();
                config.hold_click_time_threshold = data.readInt32();
                config.double_click_time_interval = data.readInt32();
                config.fast_move_tolerance = data.readInt32();
                config.slow_swipe_up_threshold = data.readInt32();
                config.slow_swipe_down_threshold = data.readInt32();
                config.slow_swipe_left_threshold = data.readInt32();
                config.slow_swipe_right_threshold = data.readInt32();
                config.fast_swipe_up_threshold = data.readInt32();
                config.fast_swipe_down_threshold = data.readInt32();
                config.fast_swipe_left_threshold = data.readInt32();
                config.fast_swipe_right_threshold = data.readInt32();
                setNavigationConfig(&config);
                reply->writeNoException();
                return NO_ERROR;
            } else if (parcel_header == NO_PARCEL_DATA) {
                ALOGE("Parcel header indicating no data, operation SET_NAVIGATION_CONFIG(%d)"
                      "cannot be performed.",
                      SET_NAVIGATION_CONFIG);
                return FAILED_TRANSACTION;
            } else {
                ALOGE("Received unrecognized parcel header: %d, operation SET_NAVIGATION_CONFIG(%d)"
                      "will not be performed.",
                      parcel_header, SET_NAVIGATION_CONFIG);
                return FAILED_TRANSACTION;
            }
        }
        case IS_ENABLED: {
            CHECK_INTERFACE(IFingerprintNavigation, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_NAVIGATION)) {
                return PERMISSION_DENIED;
            }
            bool enabled;
            isEnabled(&enabled);
            reply->writeNoException();
            reply->writeInt32(enabled ? 1 : 0);
            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintNavigation::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
