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

public class FMIBlockType {

    public enum Format {
        UNDEFINED(0),
        FPC_IMAGE_DATA(1),
        TEXT(2),
        PNG(3),
        BINARY(4),
        PROPRIETARY(5);

        private int mValue;

        private Format(int value) {
            mValue = value;
        }

        public static Format fromValue(int value) {
            for (Format f : Format.values()) {
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

    public enum Id {
        RAW_IMAGE(0),
        PREPROCESSED_IMAGE(1),
        SENSOR_OTP(32),
        SESSION(33),
        DISPLAY_IMAGE(64);

        private int mValue;

        private Id(int value) {
            mValue = value;
        }

        public static Id fromValue(int value) {
            for (Id f : Id.values()) {
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

    public Format format;
    public Id id;
}
