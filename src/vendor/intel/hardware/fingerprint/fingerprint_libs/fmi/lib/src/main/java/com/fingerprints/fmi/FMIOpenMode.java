/*
*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*
*/

package com.fingerprints.fmi;

public enum FMIOpenMode {

    READ(0),
    WRITE(1);

    private int mValue;

    private FMIOpenMode(int value) {
        mValue = value;
    }

    public static FMIOpenMode fromValue(int value) {
        for (FMIOpenMode f : FMIOpenMode.values()) {
            if (f.mValue == value) {
                return f;
            }
        }
        return null;
    }

    public int getValue() {
        return mValue;
    }
}
