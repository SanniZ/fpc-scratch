/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

public class NavigationConfig implements Parcelable {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int singleClickMinTimeThreshold;
    public int holdClickTimeThreshold;
    public int doubleClickTimeInterval;
    public int fastMoveTolerance;
    public int slowSwipeUpThreshold;
    public int slowSwipeDownThreshold;
    public int slowSwipeLeftThreshold;
    public int slowSwipeRightThreshold;
    public int fastSwipeUpThreshold;
    public int fastSwipeDownThreshold;
    public int fastSwipeLeftThreshold;
    public int fastSwipeRightThreshold;

    public NavigationConfig() {
        singleClickMinTimeThreshold = 0;
        holdClickTimeThreshold      = 0;
        doubleClickTimeInterval     = 0;
        fastMoveTolerance           = 0;
        slowSwipeUpThreshold        = 0;
        slowSwipeDownThreshold      = 0;
        slowSwipeLeftThreshold      = 0;
        slowSwipeRightThreshold     = 0;
        fastSwipeUpThreshold        = 0;
        fastSwipeDownThreshold      = 0;
        fastSwipeLeftThreshold      = 0;
        fastSwipeRightThreshold     = 0;
    }

    public NavigationConfig(NavigationConfig objToCopy) {
        this.singleClickMinTimeThreshold = objToCopy.singleClickMinTimeThreshold;
        this.holdClickTimeThreshold      = objToCopy.holdClickTimeThreshold;
        this.doubleClickTimeInterval     = objToCopy.doubleClickTimeInterval;
        this.fastMoveTolerance           = objToCopy.fastMoveTolerance;
        this.slowSwipeUpThreshold        = objToCopy.slowSwipeUpThreshold;
        this.slowSwipeDownThreshold      = objToCopy.slowSwipeDownThreshold;
        this.slowSwipeLeftThreshold      = objToCopy.slowSwipeLeftThreshold;
        this.slowSwipeRightThreshold     = objToCopy.slowSwipeRightThreshold;
        this.fastSwipeUpThreshold        = objToCopy.fastSwipeUpThreshold;
        this.fastSwipeDownThreshold      = objToCopy.fastSwipeDownThreshold;
        this.fastSwipeLeftThreshold      = objToCopy.fastSwipeLeftThreshold;
        this.fastSwipeRightThreshold     = objToCopy.fastSwipeRightThreshold;
    }

    public void print() {
        mLogger.d("singleClickMinTimeThreshold: " + singleClickMinTimeThreshold +
                  " holdClickTimeThreshold: "     + holdClickTimeThreshold +
                  " doubleClickTimeInterval: "    + doubleClickTimeInterval +
                  " fastMoveTolerance: "          + fastMoveTolerance +
                  " slowSwipeUpThreshold: "       + slowSwipeUpThreshold +
                  " slowSwipeDownThreshold: "     + slowSwipeDownThreshold +
                  " slowSwipeLeftThreshold: "     + slowSwipeLeftThreshold +
                  " slowSwipeRightThreshold: "    + slowSwipeRightThreshold +
                  " fastSwipeUpThreshold: "       + fastSwipeUpThreshold +
                  " fastSwipeDownThreshold: "     + fastSwipeDownThreshold +
                  " fastSwipeLeftThreshold: "     + fastSwipeLeftThreshold +
                  " fastSwipeRightThreshold: "    + fastSwipeRightThreshold);
    }

    private NavigationConfig(Parcel source) {
        readFromParcel(source);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public void readFromParcel(Parcel source) {
        singleClickMinTimeThreshold = source.readInt();
        holdClickTimeThreshold      = source.readInt();
        doubleClickTimeInterval     = source.readInt();
        fastMoveTolerance           = source.readInt();
        slowSwipeUpThreshold        = source.readInt();
        slowSwipeDownThreshold      = source.readInt();
        slowSwipeLeftThreshold      = source.readInt();
        slowSwipeRightThreshold     = source.readInt();
        fastSwipeUpThreshold        = source.readInt();
        fastSwipeDownThreshold      = source.readInt();
        fastSwipeLeftThreshold      = source.readInt();
        fastSwipeRightThreshold     = source.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(singleClickMinTimeThreshold);
        dest.writeInt(holdClickTimeThreshold);
        dest.writeInt(doubleClickTimeInterval);
        dest.writeInt(fastMoveTolerance);
        dest.writeInt(slowSwipeUpThreshold);
        dest.writeInt(slowSwipeDownThreshold);
        dest.writeInt(slowSwipeLeftThreshold);
        dest.writeInt(slowSwipeRightThreshold);
        dest.writeInt(fastSwipeUpThreshold);
        dest.writeInt(fastSwipeDownThreshold);
        dest.writeInt(fastSwipeLeftThreshold);
        dest.writeInt(fastSwipeRightThreshold);
    }

    public static final Parcelable.Creator<NavigationConfig> CREATOR =
            new Parcelable.Creator<NavigationConfig>() {
                @Override
                public NavigationConfig createFromParcel(Parcel source) {
                    return new NavigationConfig(source);
                }

                @Override
                public NavigationConfig[] newArray(int size) {
                    return new NavigationConfig[size];
                }
            };
}
