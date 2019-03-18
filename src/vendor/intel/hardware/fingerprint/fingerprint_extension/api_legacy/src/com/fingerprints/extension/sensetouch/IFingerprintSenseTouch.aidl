/**
* Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.sensetouch;

import com.fingerprints.extension.sensetouch.SenseTouchConfig;


/** {@hide} */
interface IFingerprintSenseTouch {
    int getForce();
    boolean isSupported();
    boolean finishCalibration(int ground, int threshold);
    boolean setAuthMode(int mode, int buttonTimeoutMs);
    boolean readConfig(out SenseTouchConfig senseTouchConfig);
}
