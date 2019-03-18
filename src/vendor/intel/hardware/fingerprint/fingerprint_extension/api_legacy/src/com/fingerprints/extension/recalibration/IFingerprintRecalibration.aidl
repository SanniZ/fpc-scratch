/**
* Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.recalibration;

import com.fingerprints.extension.recalibration.IRecalibrationCallback;

/** {@hide} */
interface IFingerprintRecalibration {
    void recalibrate(in byte[] token, IRecalibrationCallback callback);
    long preRecalibrate();
    void cancel();
}
