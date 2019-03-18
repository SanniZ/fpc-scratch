/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.util.ArrayUtils;

public class SensorTestResult {
    private Logger mLogger = new Logger(getClass().getSimpleName());

    public enum ResultCode {
        PASS(0, "PASS"),
        FAIL(1, "FAIL"),
        CANCELLED(2, "CANCELLED"),
        NOT_SUPPORTED(3, "NOT SUPPORTED"),
        ERROR(4, "ERROR");

        private int mValue;
        private String mString;

        private ResultCode(int value, String string) {
            mValue = value;
            mString = string;
        }

        public int getValue() {
            return mValue;
        }

        public String getString() {
            return mString;
        }

        public static ResultCode fromInt(int i) {
            for (ResultCode r : values()) {
                if (r.getValue() == i) {
                    return r;
                }
            }
            return ERROR;
        }
    }

    public ResultCode resultCode;
    public String resultString;
    public int errorCode;
    public String errorString;
    public boolean imageFetched;
    public byte[] image;

    public SensorTestResult(ResultCode resultCode) {
        this(resultCode, "", 0, "");
    }

    public SensorTestResult(com.fingerprints.extension.V1_0.SensorTestResult result) {
        this(ResultCode.fromInt(result.resultCode),
             result.resultString,
             result.errorCode,
             result.errorString);
        if (result.imageData != null && result.imageData.size() > 0) {
            this.imageFetched = true;
            this.image = ArrayUtils.toByteArray(result.imageData);
        }
    }

    public SensorTestResult(ResultCode resultCode, String resultString, int errorCode, String errorString) {
        mLogger.enter("SensorTestResult");
        this.resultCode = resultCode;
        this.resultString = resultString;
        this.errorCode = errorCode;
        this.errorString = errorString;
        mLogger.exit("SensorTestResult");
    }
}
