/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_CALIBRATION_H_
#define FINGERPRINT_CALIBRATION_H_

#include "IFingerprintCalibration.h"

namespace android {

class FingerprintCalibration : public BnFingerprintCalibration,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_calibration_t* device);

        static int onStatus(void* context, uint8_t code);
        static int onError(void* context, int8_t code);

        // These reflect binder methods.
        virtual void calibrate(const sp<ICalibrationCallback>& callback);

     private:
        FingerprintCalibration(fpc_calibration_t* device);
        virtual ~FingerprintCalibration();
        void binderDied(const wp<IBinder>& who);

        static FingerprintCalibration* sInstance;
        sp<ICalibrationCallback> mCalibrationCallback;
        fpc_calibration_t* mDevice;
};

}  // namespace android

#endif // FINGERPRINT_CALIBRATION_H_
