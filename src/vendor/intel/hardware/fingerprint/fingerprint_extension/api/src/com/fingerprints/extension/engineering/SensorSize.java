/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.engineering;

import com.fingerprints.extension.util.Logger;

public class SensorSize {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int mWidth;
    public int mHeight;

    public SensorSize() {
    }

    public void print() {
        mLogger.d("mWidth: " + mWidth +
                " mHeight: " + mHeight);
    }

    SensorSize(com.fingerprints.extension.V1_0.SensorSize sensorSize) {
        mWidth = sensorSize.width;
        mHeight = sensorSize.height;
    }
}
