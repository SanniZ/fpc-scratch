/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

public class SensorTestInput implements Parcelable {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public String testLimitsKeyValuePair;

    public SensorTestInput(String testLimitsKeyValuePair) {
        this.testLimitsKeyValuePair = testLimitsKeyValuePair;
    }

    private SensorTestInput(Parcel in) {
        mLogger.enter("SensorTestInput");
        try {
            testLimitsKeyValuePair = in.readString();
            mLogger.d("testLimitsKeyValuePair: " + testLimitsKeyValuePair);
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }
        mLogger.exit("SensorTestInput");
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        mLogger.enter("writeToParcel");
        mLogger.d("testLimitsKeyValuePair: " + testLimitsKeyValuePair);
        dest.writeString(testLimitsKeyValuePair);
        mLogger.exit("writeToParcel");
    }

    public static final Parcelable.Creator<SensorTestInput> CREATOR =
            new Parcelable.Creator<SensorTestInput>() {
                @Override
                public SensorTestInput createFromParcel(Parcel in) {
                    return new SensorTestInput(in);
                }

                @Override
                public SensorTestInput[] newArray(int size) {
                    return new SensorTestInput[size];
                }
            };
}
