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

public enum FMIStatus {
    OK(0),
    FAILED_MALLOC(1),
    FAILED_IO(2),
    INVALID_ARGUMENT(3),
    INVALID_CONTEXT(4),
    STORAGE_IS_FULL(5),
    UNSUPPORTED(6),
    NOT_FOUND(7);

    private int mValue;

    private FMIStatus(int value) {
        mValue = value;
    }

    public static FMIStatus fromValue(int value) {
        for (FMIStatus f : FMIStatus.values()) {
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
