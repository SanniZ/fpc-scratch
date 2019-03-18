/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_recalibration_H_
#define IFINGERPRINT_recalibration_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_recalibration.h"

namespace android {

class IRecalibrationCallback : public IInterface {
    public:
    enum {
       ON_STATUS = IBinder::FIRST_CALL_TRANSACTION,
       ON_ERROR,
    };
    // must be kept in sync with IRecalibrationCallback.aidl
    virtual void onStatus(int32_t code, int32_t imageDecision, int32_t imageQuality,
            int32_t pnQuality, int32_t progress) = 0;
    virtual void onError(int32_t code) = 0;

    DECLARE_META_INTERFACE (RecalibrationCallback);
};

class IFingerprintRecalibration : public IInterface {
     public:
        enum {
            RECALIBRATE = IBinder::FIRST_CALL_TRANSACTION,
            PRERECALIBRATE,
            CANCEL,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void recalibrate(const uint8_t* token, ssize_t tokenLength,
                const sp<IRecalibrationCallback>& callback) = 0;
        virtual void preRecalibrate(uint64_t* challenge) = 0;

        virtual void cancel() = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintRecalibration : public BnInterface<IFingerprintRecalibration> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);

     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_recalibration_H_

