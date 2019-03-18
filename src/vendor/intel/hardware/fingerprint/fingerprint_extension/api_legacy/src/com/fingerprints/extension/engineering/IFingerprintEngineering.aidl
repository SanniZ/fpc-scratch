/**
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.engineering;

import com.fingerprints.extension.engineering.SensorSize;
import com.fingerprints.extension.engineering.IImageSubscriptionCallback;
import com.fingerprints.extension.engineering.IImageInjectionCallback;
import com.fingerprints.extension.engineering.ICaptureCallback;

/** {@hide} */
interface IFingerprintEngineering {
    SensorSize getSensorSize();
    void startImageSubscription(IImageSubscriptionCallback callback);
    void stopImageSubscription();
    void startImageInjection(IImageInjectionCallback callback);
    void stopImageInjection();
    void startCapture(ICaptureCallback callback, int mode);
    void cancelCapture();
    void setEnrollToken(in byte[] token);
    long getEnrollChallenge();
}
