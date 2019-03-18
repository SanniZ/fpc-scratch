/**
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.sensortest;

import com.fingerprints.extension.sensortest.SensorInfo;
import com.fingerprints.extension.sensortest.SensorTest;
import com.fingerprints.extension.sensortest.SensorTestInput;
import com.fingerprints.extension.sensortest.ISensorTestCallback;
import com.fingerprints.extension.sensortest.ICaptureCallback;

/** {@hide} */
interface IFingerprintSensorTest {
    SensorInfo getSensorInfo();
    List<SensorTest> getSensorTests();
    void runSensorTest(ISensorTestCallback callback, in SensorTest test, in SensorTestInput input);
    void cancelSensorTest();
    void capture(ICaptureCallback callback, in boolean waitForFinger, in boolean uncalibrated);
    void cancelCapture();
}
