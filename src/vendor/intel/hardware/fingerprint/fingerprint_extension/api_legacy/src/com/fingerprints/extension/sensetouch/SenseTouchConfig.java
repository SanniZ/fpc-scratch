/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensetouch;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

public class SenseTouchConfig implements Parcelable {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int version;
    public int ground;
    public int triggerThreshold;
    public int untriggerThreshold;
    public boolean authTriggerOnDown;
    public boolean authTriggerOnUp;
    public int authButtonTimeout;

    public SenseTouchConfig() {
        mLogger.enter("SenseTouchConfig()");
        version            = 0;
        ground             = 0;
        triggerThreshold   = 0;
        untriggerThreshold = 0;
        authTriggerOnDown  = false;
        authTriggerOnUp    = false;
        authButtonTimeout  = 0;
        mLogger.exit("SenseTouchConfig()");
    }

    public SenseTouchConfig(SenseTouchConfig objToCopy) {
        mLogger.enter("SenseTouchConfig(SenseTouchConfig)");
        this.version            = objToCopy.version;
        this.ground             = objToCopy.ground;
        this.triggerThreshold   = objToCopy.triggerThreshold;
        this.untriggerThreshold = objToCopy.untriggerThreshold;
        this.authTriggerOnDown  = objToCopy.authTriggerOnDown;
        this.authTriggerOnUp    = objToCopy.authTriggerOnUp;
        this.authButtonTimeout  = objToCopy.authButtonTimeout;
        mLogger.exit("SenseTouchConfig()");
    }

    public void print() {
        mLogger.enter("print()");
        mLogger.d("version: "             + version +
                  " ground: "             + ground +
                  " triggerThreshold: "   + triggerThreshold +
                  " untriggerThreshold: " + untriggerThreshold +
                  " authTriggerOnDown: "  + authTriggerOnDown +
                  " authTriggerOnUp: "    + authTriggerOnUp +
                  " authButtonTimeout: "  + authButtonTimeout);
        mLogger.exit("print()");
    }

    private SenseTouchConfig(Parcel source) {
        mLogger.enter("SenseTouchConfig(Parcel)");
        readFromParcel(source);
        mLogger.exit("SenseTouchConfig()");
    }

    @Override
    public int describeContents() {
        mLogger.enter("describeContents()");
        mLogger.exit("describeContents()");
        return 0;
    }

    public void readFromParcel(Parcel source) {
        mLogger.enter("readFromParcel(Parcel)");
        version            = source.readInt();
        ground             = source.readInt();
        triggerThreshold   = source.readInt();
        untriggerThreshold = source.readInt();
        authTriggerOnDown  = (source.readInt() != 0);
        authTriggerOnUp    = (source.readInt() != 0);
        authButtonTimeout  = source.readInt();
        mLogger.exit("readFromParcel()");
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        mLogger.enter("writeToParcel(Parcel, flags: " + flags + ")");
        dest.writeInt(version);
        dest.writeInt(ground);
        dest.writeInt(triggerThreshold);
        dest.writeInt(untriggerThreshold);
        dest.writeInt(authTriggerOnDown ? 1 : 0);
        dest.writeInt(authTriggerOnUp ? 1 : 0);
        dest.writeInt(authButtonTimeout);
        mLogger.exit("writeToParcel()");
    }

    public static final Parcelable.Creator<SenseTouchConfig> CREATOR =
        new Parcelable.Creator<SenseTouchConfig>() {
            @Override
            public SenseTouchConfig createFromParcel(Parcel source) {
                return new SenseTouchConfig(source);
            }

            @Override
            public SenseTouchConfig[] newArray(int size) {
                return new SenseTouchConfig[size];
            }
        };
}
