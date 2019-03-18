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
#include "IFingerprintEngineering.h"

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

static const String16 FINGERPRINT_EXTENSION_ENGINEERING(
        "com.fingerprints.extension.ENGINEERING");

/***** ICaptureCallback *****/
class BpCaptureCallback: public BpInterface<ICaptureCallback> {
    public:
        BpCaptureCallback(const sp<IBinder>& impl) :
                BpInterface<ICaptureCallback>(impl) {
        }

        virtual void onCapture(fpc_capture_data_t *capture_data) {
            Parcel data, reply;
            data.writeInterfaceToken(ICaptureCallback::getInterfaceDescriptor());

            data.writeInt32(capture_data->mode);
            data.writeInt32(capture_data->capture_result);
            data.writeInt32(capture_data->identify_result);
            data.writeInt32(capture_data->template_update_result);
            data.writeInt32(capture_data->enroll_result);
            data.writeInt32(capture_data->cac_result);
            data.writeInt32(capture_data->user_id);
            data.writeInt32(capture_data->samples_remaining);
            data.writeInt32(capture_data->coverage);
            data.writeInt32(capture_data->quality);
            data.writeByteArray(capture_data->raw_image.buffer_size,
                                capture_data->raw_image.buffer);
            data.writeByteArray(capture_data->enhanced_image.buffer_size,
                                capture_data->enhanced_image.buffer);
            status_t status = remote()->transact(ON_CAPTURE, data, &reply,
                    IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
                ALOGD("onResult() could not contact remote: %d\n", status);
            }
        }
};

IMPLEMENT_META_INTERFACE(CaptureCallback,
        "com.fingerprints.extension.engineering.ICaptureCallback");

/***** IImageSubscriptionCallback *****/

class BpImageSubscriptionCallback : public BpInterface<
        IImageSubscriptionCallback> {
     public:
        BpImageSubscriptionCallback(const sp<IBinder>& impl)
                : BpInterface<IImageSubscriptionCallback>(impl) {
        }

        virtual void onImage(fpc_capture_data_t *capture_data) {
            Parcel data, reply;
            data.writeInterfaceToken(IImageSubscriptionCallback::getInterfaceDescriptor());

            data.writeInt32(capture_data->mode);
            data.writeInt32(capture_data->capture_result);
            data.writeInt32(capture_data->identify_result);
            data.writeInt32(capture_data->template_update_result);
            data.writeInt32(capture_data->enroll_result);
            data.writeInt32(capture_data->cac_result);
            data.writeInt32(capture_data->user_id);
            data.writeInt32(capture_data->samples_remaining);
            data.writeInt32(capture_data->coverage);
            data.writeInt32(capture_data->quality);
            data.writeByteArray(capture_data->raw_image.buffer_size,
                                capture_data->raw_image.buffer);
            data.writeByteArray(capture_data->enhanced_image.buffer_size,
                                capture_data->enhanced_image.buffer);
            status_t status = remote()->transact(ON_IMAGE, data, &reply,
                    IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
                ALOGD("onImage() could not contact remote: %d\n", status);
            }

        }
};

IMPLEMENT_META_INTERFACE(ImageSubscriptionCallback,
        "com.fingerprints.extension.engineering.IImageSubscriptionCallback");

/***** IImageInjectionCallback *****/

class BpImageInjectionCallback : public BpInterface<IImageInjectionCallback> {
     public:
        BpImageInjectionCallback(const sp<IBinder>& impl)
                : BpInterface<IImageInjectionCallback>(impl) {
        }

        virtual void onInject(fpc_hal_img_data_t* img_data) {
            Parcel data, reply;
            data.writeInterfaceToken(
                    IImageInjectionCallback::getInterfaceDescriptor());
            status_t status = remote()->transact(ON_INJECT, data, &reply);
            if (status != NO_ERROR) {
                ALOGD("onInject() could not contact remote: %d\n", status);
            }
            uint8_t** image = &(img_data->buffer);
            uint32_t* imageSize = &(img_data->buffer_size);

            int32_t error = reply.readExceptionCode();
            ssize_t length = reply.readInt32();
            if (length > 0 && (size_t) length <= reply.dataAvail()) {
                size_t ulength = (size_t) length;
                const void* buf = reply.readInplace(ulength);
                *image = (uint8_t*) malloc(ulength);
                if (*image != NULL) {
                    memcpy(*image, buf, ulength);
                    *imageSize = ulength;
                } else {
                    ALOGE("out of memory");
                    *imageSize = 0;
                }
            } else {
                *imageSize = 0;
                *image = NULL;
            }
            if (error < 0) {
                ALOGD("onInject() caught exception %d\n", error);
            }
        }

        virtual void onCancel() {
            Parcel data, reply;
            data.writeInterfaceToken(
                    IImageInjectionCallback::getInterfaceDescriptor());
            status_t status = remote()->transact(ON_CANCEL, data, &reply,
                                                 IBinder::FLAG_ONEWAY);
            if (status != NO_ERROR) {
                ALOGD("onCancel() could not contact remote: %d\n", status);
            }
        }

};

IMPLEMENT_META_INTERFACE(ImageInjectionCallback,
        "com.fingerprints.extension.engineering.IImageInjectionCallback");

/***** IFingerprintEngineering *****/

const android::String16 IFingerprintEngineering::descriptor(
        "com.fingerprints.extension.engineering.IFingerprintEngineering");

const android::String16& IFingerprintEngineering::getInterfaceDescriptor() const {
    return IFingerprintEngineering::descriptor;
}

status_t BnFingerprintEngineering::onTransact(uint32_t code, const Parcel& data,
                                              Parcel* reply, uint32_t flags) {
    switch (code) {
        case GET_SENSOR_SIZE: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            uint8_t width;
            uint8_t height;
            getSensorSize(&width, &height);
            reply->writeNoException();
            reply->writeInt32(HAS_PARCEL_DATA);
            reply->writeInt32(width);
            reply->writeInt32(height);
            return NO_ERROR;
        }
        case START_IMAGE_SUBSCRIPTION: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            sp < IImageSubscriptionCallback > callback = interface_cast
                    < IImageSubscriptionCallback > (data.readStrongBinder());
            startImageSubscription (callback);
            reply->writeNoException();
            return NO_ERROR;
        }
        case STOP_IMAGE_SUBSCRIPTION: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            stopImageSubscription();
            reply->writeNoException();
            return NO_ERROR;
        }
        case START_IMAGE_INJECTION: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            sp < IImageInjectionCallback > callback = interface_cast
                    < IImageInjectionCallback > (data.readStrongBinder());
            startImageInjection (callback);
            reply->writeNoException();
            return NO_ERROR;
        }
        case STOP_IMAGE_INJECTION: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            stopImageInjection();
            reply->writeNoException();
            return NO_ERROR;
        }
        case START_CAPTURE: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }

            sp < ICaptureCallback > callback = interface_cast
                                < ICaptureCallback > (data.readStrongBinder());

            fpc_capture_mode_t mode = (fpc_capture_mode_t)data.readInt32();

            startCapture(callback, mode);

            reply->writeNoException();
            return NO_ERROR;
        }
        case CANCEL_CAPTURE: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }
            cancelCapture();
            reply->writeNoException();
            return NO_ERROR;
        }
        case SET_ENROLL_TOKEN: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }

            ssize_t tokenLength = data.readInt32();
            const uint8_t* token = static_cast<const uint8_t *>(data.readInplace(
                    tokenLength));
            setEnrollToken(token, tokenLength);
            reply->writeNoException();

            return NO_ERROR;
        }
        case GET_ENROLL_CHALLENGE: {
            CHECK_INTERFACE(IFingerprintEngineering, data, reply);
            if (!checkPermission(FINGERPRINT_EXTENSION_ENGINEERING)) {
                return PERMISSION_DENIED;
            }

            uint64_t challenge;

            getEnrollChallenge(&challenge);
            reply->writeNoException();
            reply->writeInt64(challenge);

            return NO_ERROR;
        }
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

bool BnFingerprintEngineering::checkPermission(const String16& permission) {
    const IPCThreadState* ipc = IPCThreadState::self();
    const int calling_pid = ipc->getCallingPid();
    const int calling_uid = ipc->getCallingUid();
    return PermissionCache::checkPermission(permission, calling_pid,
                                            calling_uid);
}

}
// namespace android
