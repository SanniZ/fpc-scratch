/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import com.fingerprints.extension.util.Logger;

public class SensorTest {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public String name;
    public String description;
    public boolean waitForFingerDown;
    public String rubberStampType;

    public SensorTest(String name, String description, boolean waitForFingerDown,
                      String rubberStampType) {
        mLogger.enter("SensorTest");
        this.name = name;
        this.description = description;
        this.waitForFingerDown = waitForFingerDown;
        this.rubberStampType = rubberStampType;
        mLogger.exit("SensorTest");
    }

    SensorTest(com.fingerprints.extension.V1_0.SensorTest test) {
        this(test.name, test.description, test.waitForFingerDown, test.rubberStampType);
    }

    com.fingerprints.extension.V1_0.SensorTest getHidl() {
        com.fingerprints.extension.V1_0.SensorTest sensorTest =
                new com.fingerprints.extension.V1_0.SensorTest();
        sensorTest.name = this.name;
        sensorTest.description = this.description;
        sensorTest.waitForFingerDown = this.waitForFingerDown;
        sensorTest.rubberStampType = this.rubberStampType;
        return sensorTest;
    }
}
