/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_ENGINEERING_H_
#define FINGERPRINT_ENGINEERING_H_

#include "IFingerprintEngineering.h"

namespace android {

class FingerprintEngineering : public BnFingerprintEngineering,
        public IBinder::DeathRecipient {
     public:
        static void instantiate(fpc_engineering_t* device);
        static void onImage(void* context, fpc_capture_data_t* capture_data);
        static int onInject(void* context, fpc_hal_img_data_t* img_data);
        static void onCancel(void* context);
        static void onCapture(void* context, fpc_capture_data_t* capture_data);

        // These reflect binder methods.
        virtual void getSensorSize(uint8_t* width, uint8_t* height);
        virtual void startImageSubscription(
                const sp<IImageSubscriptionCallback>& callback);
        virtual void stopImageSubscription();
        virtual void startImageInjection(
                const sp<IImageInjectionCallback>& callback);
        virtual void stopImageInjection();
        virtual void startCapture(const sp<ICaptureCallback>& callback, fpc_capture_mode_t mode);
        virtual void cancelCapture();
        virtual void setEnrollToken(const uint8_t* token, ssize_t tokenLength);
        virtual void getEnrollChallenge(uint64_t* challenge);

     private:
        FingerprintEngineering(fpc_engineering_t* device);
        virtual ~FingerprintEngineering();
        void binderDied(const wp<IBinder>& who);

        static FingerprintEngineering* sInstance;
        sp<IImageSubscriptionCallback> mImageSubscriptionCallback;
        sp<IImageInjectionCallback> mImageInjectionCallback;
        sp<ICaptureCallback> mCaptureCallback;
        fpc_engineering_t* mDevice;
        bool isCapture;
};

}  // namespace android

#endif // FINGERPRINT_ENGINEERING_H_
