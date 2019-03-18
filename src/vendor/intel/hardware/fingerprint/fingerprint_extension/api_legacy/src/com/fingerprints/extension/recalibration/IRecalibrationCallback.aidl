/**
* Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.recalibration;

/** {@hide} */
interface IRecalibrationCallback {
    void onStatus(int code, boolean imageDecision, int imageQuality, int pnQuality, int progress);
    void onError(int error);
}