/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_calibration_H_
#define IFINGERPRINT_calibration_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_calibration.h"

namespace android {

class ICalibrationCallback : public IInterface {
    public:
    enum {
       ON_CALIBRATION_STATUS = IBinder::FIRST_CALL_TRANSACTION,
       ON_CALIBRATION_ERROR,
    };
    // must be kept in sync with ICalibrationCallback.aidl
    virtual void onStatus(uint8_t code) = 0;
    virtual void onError(int8_t code) = 0;

    DECLARE_META_INTERFACE (CalibrationCallback);
};

class IFingerprintCalibration : public IInterface {
     public:
        enum {
            CALIBRATE = IBinder::FIRST_CALL_TRANSACTION,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void calibrate(const sp<ICalibrationCallback>& callback) = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintCalibration : public BnInterface<IFingerprintCalibration> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);

     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_calibration_H_

