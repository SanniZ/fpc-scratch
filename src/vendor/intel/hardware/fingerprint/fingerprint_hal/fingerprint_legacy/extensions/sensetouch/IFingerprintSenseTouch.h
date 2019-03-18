/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_sense_touch_H_
#define IFINGERPRINT_sense_touch_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_sense_touch.h"
#include "fpc_hal_sense_touch_types.h"

namespace android {

//Needs to be in sync with FingerprintSenseTouch.java
enum class SenseTouchAuthenticationMode
{
    AUTH_MODE_NORMAL,
    AUTH_MODE_ON_FORCE_TRIGGER,
    AUTH_MODE_ON_FORCE_RELEASE
};

class IFingerprintSenseTouch : public IInterface {
     public:
        enum {
            GET_FORCE = IBinder::FIRST_CALL_TRANSACTION,
            IS_SUPPORTED,
            FINISH_CALIBRATION,
            SET_AUTH_MODE,
            READ_CONFIG,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual int getForce(uint8_t* force) = 0;
        virtual int isSupported(uint8_t* result) = 0;
        virtual int storeCalibrationData(uint8_t ground, uint8_t threshold) = 0;
        virtual int setAuthMode(SenseTouchAuthenticationMode mode,
                                uint32_t button_timeout_ms) = 0;
        virtual int readConfig(const fpc_sense_touch_config_t** senseTouchConfig) = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintSenseTouch : public BnInterface<IFingerprintSenseTouch> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);

     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_sense_touch_H_

