/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.util;

import android.os.Build;
import android.util.Log;

import java.lang.reflect.Method;

public class Logger {
    private static final int NONE = -1;
    private static final int ERROR = 0;
    private static final int WARNING = 1;
    private static final int INFO = 2;
    private static final int DEBUG = 3;
    private static final int VERBOSE = 4;
    private static final int TRACE = 5;
    private static final String TAG = "FpcExtension";
    private static final String PROPERTY_FINGERPRINTS_DEBUG = "persist.fingerprints.dbg.level";
    private static final int DEFAULT_LOG_LEVEL = 5;
    private final String mClassName;
    private final int mLogLevel;

    public Logger(String s) {
        mClassName = s;
        mLogLevel = getLogLevel();
    }

    private int getLogLevel() {
        if ("user".equals(Build.TYPE)) {
            return WARNING;
        } else {
            try {
                final Class systemProperties = Class.forName("android.os.SystemProperties");
                if (systemProperties != null) {
                    final Method getInt = systemProperties.getMethod("getInt", String.class, int.class);
                    if (getInt != null) {
                        return (int) getInt.invoke(null, PROPERTY_FINGERPRINTS_DEBUG, DEFAULT_LOG_LEVEL);
                    }
                }
            } catch (Exception ignore) {
            }
            return DEFAULT_LOG_LEVEL;
        }
    }

    private boolean atERROR() {
        return ERROR <= mLogLevel;
    }

    private boolean atWARNING() {
        return WARNING <= mLogLevel;
    }

    private boolean atINFO() {
        return INFO <= mLogLevel;
    }

    private boolean atVERBOSE() {
        return VERBOSE <= mLogLevel;
    }

    private boolean atDEBUG() {
        return DEBUG <= mLogLevel;
    }

    private boolean atTRACE() {
        return TRACE <= mLogLevel;
    }

    public void e(String s) {
        if (atERROR()) {
            Log.e(TAG, mClassName + ":" + s);
        }
    }

    public void e(String s, Throwable t) {
        if (atERROR()) {
            Log.e(TAG, mClassName + ":" + s, t);
        }
    }

    public void w(String s) {
        if (atWARNING()) {
            Log.w(TAG, mClassName + ":" + s);
        }
    }

    public void w(String s, Throwable t) {
        if (atWARNING()) {
            Log.w(TAG, mClassName + ":" + s, t);
        }
    }

    public void i(String s) {
        if (atINFO()) {
            Log.i(TAG, mClassName + ":" + s);
        }
    }

    public void i(String s, Throwable t) {
        if (atINFO()) {
            Log.i(TAG, mClassName + ":" + s, t);
        }
    }

    public void d(String s) {
        if (atDEBUG()) {
            Log.d(TAG, mClassName + ":" + s);
        }
    }

    public void d(String s, Throwable t) {
        if (atDEBUG()) {
            Log.d(TAG, mClassName + ":" + s, t);
        }
    }

    public void v(String s) {
        if (atVERBOSE()) {
            Log.v(TAG, mClassName + ":" + s);
        }
    }

    public void v(String s, Throwable t) {
        if (atVERBOSE()) {
            Log.v(TAG, mClassName + ":" + s, t);
        }
    }

    public void enter(String s) {
        if (atTRACE()) {
            Log.v(TAG, mClassName + ":" + s + " +");
        }
    }

    public void exit(String s) {
        if (atTRACE()) {
            Log.v(TAG, mClassName + ":" + s + " -");
        }
    }
}
