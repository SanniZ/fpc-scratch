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

package com.fingerprints.extension.common;

import android.os.IBinder;

import com.fingerprints.extension.util.Logger;

import java.lang.reflect.Method;

public class FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());

    public FingerprintExtensionBase() {
    }

    public IBinder getFingerprintExtension(String extension) {
        mLogger.enter("getFingerprintExtension");
        try {
            Class<?> serviceManagerClazz = Class.forName("android.os.ServiceManager");
            Method getService = serviceManagerClazz.getDeclaredMethod("getService", String.class);
            return (IBinder) getService.invoke(null, extension);
        } catch (Exception e) {
            mLogger.w("Exception: " + e);
        }
        mLogger.exit("getFingerprintExtension");
        return null;
    }
}
