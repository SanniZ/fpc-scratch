/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.engineering;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

public class SensorSize implements Parcelable {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int mWidth;
    public int mHeight;

    public SensorSize() {
    }

    public void print() {
        mLogger.d("mWidth: " + mWidth +
                " mHeight: " + mHeight);
    }

    private SensorSize(Parcel in) {
        mWidth = in.readInt();
        mHeight = in.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mWidth);
        dest.writeInt(mHeight);
    }

    public static final Parcelable.Creator<SensorSize> CREATOR =
            new Parcelable.Creator<SensorSize>() {
                @Override
                public SensorSize createFromParcel(Parcel in) {
                    return new SensorSize(in);
                }

                @Override
                public SensorSize[] newArray(int size) {
                    return new SensorSize[size];
                }
            };
}
