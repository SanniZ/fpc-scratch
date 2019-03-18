/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import com.fingerprints.extension.util.Logger;

public class SensorTestInput {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public String testLimitsKeyValuePair;

    public SensorTestInput(String testLimitsKeyValuePair) {
        this.testLimitsKeyValuePair = testLimitsKeyValuePair;
    }

    com.fingerprints.extension.V1_0.SensorTestInput getHidl() {
        com.fingerprints.extension.V1_0.SensorTestInput input =
                new com.fingerprints.extension.V1_0.SensorTestInput();
        input.testLimitsKeyValuePair = this.testLimitsKeyValuePair;
        return input;
    }
}
