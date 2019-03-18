/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_ENGINEERING_H_
#define IFINGERPRINT_ENGINEERING_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_engineering.h"

namespace android {

class IImageSubscriptionCallback : public IInterface {
     public:
        // must be kept in sync with IImageSubscriptionCallback.aidl
        enum {
            ON_IMAGE = IBinder::FIRST_CALL_TRANSACTION,
        };

        virtual void onImage(fpc_capture_data_t *capture_data) = 0;

        DECLARE_META_INTERFACE (ImageSubscriptionCallback);
};

class IImageInjectionCallback : public IInterface {
     public:
        // must be kept in sync with IImageInjectionCallback.aidl
        enum {
            ON_INJECT = IBinder::FIRST_CALL_TRANSACTION,
            ON_CANCEL,
        };

        virtual void onInject(fpc_hal_img_data_t* img_data) = 0;
        virtual void onCancel() = 0;

        DECLARE_META_INTERFACE (ImageInjectionCallback);
};

class ICaptureCallback: public IInterface {
public:
    // must be kept in sync with ICaptureCallback.aidl
    enum {
        ON_CAPTURE = IBinder::FIRST_CALL_TRANSACTION,
    };

    virtual void onCapture(fpc_capture_data_t *capture_data) = 0;

    DECLARE_META_INTERFACE (CaptureCallback);
};

class IFingerprintEngineering : public IInterface {
     public:
        enum {
            GET_SENSOR_SIZE = IBinder::FIRST_CALL_TRANSACTION,
            START_IMAGE_SUBSCRIPTION,
            STOP_IMAGE_SUBSCRIPTION,
            START_IMAGE_INJECTION,
            STOP_IMAGE_INJECTION,
            START_CAPTURE,
            CANCEL_CAPTURE,
            SET_ENROLL_TOKEN,
            GET_ENROLL_CHALLENGE,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void getSensorSize(uint8_t* width, uint8_t* height) = 0;
        virtual void startImageSubscription(
                const sp<IImageSubscriptionCallback>& callback) = 0;
        virtual void stopImageSubscription() = 0;
        virtual void startImageInjection(
                const sp<IImageInjectionCallback>& callback) = 0;
        virtual void stopImageInjection() = 0;
        virtual void startCapture(
                const sp<ICaptureCallback>& callback, fpc_capture_mode_t mode) = 0;
        virtual void cancelCapture() = 0;
        virtual void setEnrollToken(const uint8_t* token, ssize_t tokenLength) = 0;
        virtual void getEnrollChallenge(uint64_t* challenge) = 0;
        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintEngineering : public BnInterface<IFingerprintEngineering> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);

     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_ENGINEERING_H_

