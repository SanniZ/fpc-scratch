/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.util;

import java.util.ArrayList;

public final class ArrayUtils {
    public static ArrayList<Byte> toArrayList(byte[] array) {
        ArrayList<Byte> arrayList = new ArrayList<Byte>(array.length);
        for (byte b : array) {
            arrayList.add(b);
        }
        return arrayList;
    }

    public static byte[] toByteArray(ArrayList<Byte> arrayList) {
        byte[] byteArray = new byte[arrayList.size()];
        int i=0;
        for (byte b : arrayList) {
            byteArray[i++] = b;
        }
        return byteArray;
    }
}