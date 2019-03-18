/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

public class SensorTestResult implements Parcelable {
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

    public SensorTestResult(ResultCode resultCode, String resultString, int errorCode, String errorString) {
        mLogger.enter("SensorTestResult");
        this.resultCode = resultCode;
        this.resultString = resultString;
        this.errorCode = errorCode;
        this.errorString = errorString;
        mLogger.exit("SensorTestResult");
    }

    private SensorTestResult(Parcel in) {
        mLogger.enter("SensorTestResult");
        try {
            resultCode = ResultCode.fromInt(in.readInt());
            resultString = in.readString();
            errorCode = in.readInt();
            errorString = in.readString();
            imageFetched = in.readInt() != 0;
            if (imageFetched) {
                image = in.createByteArray();
            }
            mLogger.d("resultCode: " + resultCode);
            mLogger.d("resultString: " + resultString);
            mLogger.d("errorCode: " + errorCode);
            mLogger.d("errorString: " + errorString);
            mLogger.d(String.format("imageFetched: %b",imageFetched));
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }
        mLogger.exit("SensorTestResult");
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        mLogger.enter("writeToParcel");
        mLogger.d("resultCode: " + resultCode);
        mLogger.d("resultString: " + resultString);
        mLogger.d("errorCode: " + errorCode);
        mLogger.d("errorString: " + errorString);
        dest.writeInt(resultCode.getValue());
        dest.writeString(resultString);
        dest.writeInt(errorCode);
        dest.writeString(errorString);
        mLogger.exit("writeToParcel");
    }

    public static final Parcelable.Creator<SensorTestResult> CREATOR =
            new Parcelable.Creator<SensorTestResult>() {
                @Override
                public SensorTestResult createFromParcel(Parcel in) {
                    return new SensorTestResult(in);
                }

                @Override
                public SensorTestResult[] newArray(int size) {
                    return new SensorTestResult[size];
                }
            };
}
