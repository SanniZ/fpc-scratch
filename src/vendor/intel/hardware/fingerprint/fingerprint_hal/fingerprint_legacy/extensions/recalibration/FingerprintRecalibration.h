/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_RECALIBRATION_H_
#define FINGERPRINT_RECALIBRATION_H_

#include "IFingerprintRecalibration.h"

namespace android {

class FingerprintRecalibration : public BnFingerprintRecalibration,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_recalibration_t* device);

        static void onStatus(void* context, int32_t code, int32_t imageDecision,
                int32_t imageQuality, int32_t pnQuality, int32_t progress);
        static void onError(void* context, int32_t code);

        // These reflect binder methods.
        virtual void recalibrate(const uint8_t* token, ssize_t tokenLength,
                const sp<IRecalibrationCallback>& callback);
        virtual void preRecalibrate(uint64_t* challenge);
        virtual void cancel();
     private:
        FingerprintRecalibration(fpc_recalibration_t* device);
        virtual ~FingerprintRecalibration();
        void binderDied(const wp<IBinder>& who);

        static FingerprintRecalibration* sInstance;
        sp<IRecalibrationCallback> mRecalibrationCallback;
        fpc_recalibration_t* mDevice;
};

}  // namespace android

#endif // FINGERPRINT_RECALIBRATION_H_
