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

public class SensorTest implements Parcelable {
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

    private SensorTest(Parcel in) {
        mLogger.enter("SensorTest");
        try {
            name = in.readString();
            description = in.readString();
            waitForFingerDown = in.readInt() != 0;
            rubberStampType = in.readString();
            mLogger.d("name: " + name);
            mLogger.d("description: " + description);
            mLogger.d("waitForFingerDown: " + waitForFingerDown);
            mLogger.d("rubberStampType: " + rubberStampType);
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }
        mLogger.exit("SensorTest");
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        mLogger.enter("writeToParcel");
        mLogger.d("name: " + name);
        mLogger.d("description: " + description);
        mLogger.d("waitForFingerDown: " + waitForFingerDown);
        mLogger.d("rubberStampType: " + rubberStampType);
        dest.writeString(name);
        dest.writeString(description);
        dest.writeInt((int) (waitForFingerDown ? 1 : 0));
        dest.writeString(rubberStampType);
        mLogger.exit("writeToParcel");
    }

    public static final Parcelable.Creator<SensorTest> CREATOR =
            new Parcelable.Creator<SensorTest>() {
                @Override
                public SensorTest createFromParcel(Parcel in) {
                    return new SensorTest(in);
                }

                @Override
                public SensorTest[] newArray(int size) {
                    return new SensorTest[size];
                }
            };
}
