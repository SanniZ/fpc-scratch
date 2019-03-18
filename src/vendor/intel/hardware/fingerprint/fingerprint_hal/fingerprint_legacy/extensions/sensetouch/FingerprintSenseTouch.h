/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_SENSE_TOUCH_H_
#define FINGERPRINT_SENSE_TOUCH_H_

#include "IFingerprintSenseTouch.h"
#include "fpc_hal_sense_touch_types.h"

namespace android {

class FingerprintSenseTouch : public BnFingerprintSenseTouch,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_sense_touch_t* device);

        virtual int getForce(uint8_t* force);
        virtual int isSupported(uint8_t* result);
        virtual int storeCalibrationData(uint8_t ground, uint8_t threshold);
        virtual int setAuthMode(SenseTouchAuthenticationMode mode,
                                uint32_t button_timeout_ms);
        virtual int readConfig(const fpc_sense_touch_config_t** senseTouchConfig);

     private:
        FingerprintSenseTouch(fpc_sense_touch_t* device);
        virtual ~FingerprintSenseTouch();
        void binderDied(const wp<IBinder>& who);

        static FingerprintSenseTouch* sInstance;
        fpc_sense_touch_t* mDevice;
};

}  // namespace android

#endif // FINGERPRINT_SENSE_TOUCH_H_
