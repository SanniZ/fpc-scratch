/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
package com.fingerprints.extension.sensetouch;

import android.os.RemoteException;

import com.fingerprints.extension.common.CanceledException;
import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.sensetouch.SenseTouchConfig;
import com.fingerprints.extension.V1_0.IFingerprintSenseTouch;

import java.util.ArrayList;

public class FingerprintSenseTouch {
    private static final int MAX_FORCE = 255;
    private static final int NUMBER_AVERAGE_VALUES = 20;
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String SENSE_TOUCH = "com.fingerprints.extension.sensetouch.IFingerprintSenseTouch";
    private IFingerprintSenseTouch mFingerprintSenseTouch;
    private volatile boolean mCancel;

    public enum AuthMode {
        /**
         * Authenticate when touching sensor, no pressure needed on sensor.
         */
        NORMAL,
        /**
         * Authenticate when force is above calibrated value.
         */
        ON_FORCE_TRIGGER,
        /**
         * Authenticate when force is first above calibrated value, then below
         * calibrated value times FPC_HAL_SENSE_TOUCH_UNTRIGGER_MODIFIER.
         */
        ON_FORCE_RELEASE;
    }

    public FingerprintSenseTouch() throws RemoteException {
        mLogger.enter("FingerprintSenseTouch");
        mFingerprintSenseTouch = IFingerprintSenseTouch.getService();
        if (mFingerprintSenseTouch == null) {
            throw new RemoteException("Could not get IFingerprintSenseTouch service");
        }
        mLogger.exit("FingerprintSenseTouch");
    }

    /**
     * Sets authentication mode
     *
     * @param mode
     * @param buttonTimeoutMs
     * @return true if the mode was successfully set, false otherwise.
     */
    public boolean setAuthMode(AuthMode mode, int buttonTimeoutMs) {
        if (mFingerprintSenseTouch != null) {
            try {
                return mFingerprintSenseTouch.setAuthMode(mode.ordinal(), buttonTimeoutMs);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        return false;
    }

    /**
     * Calibrates one value. Is a blocking call and should not be called from
     * main thread.
     *
     * @param time
     *            Stable time needed to finish calibration
     * @param sensitivity
     *            If the value fluctuates above or below the given sensitivity,
     *            the calibration will reset.
     * @param callback
     *            callback
     * @return the calibrated value
     */
    public float calibrate(long time, float sensitivity, float minimumValue, final CalibrationCallback callback)
            throws CanceledException {

        float value;
        float previousNormalizedValue = 0.0f;
        long startTime = System.currentTimeMillis();

        ArrayList<Float> mValues = new ArrayList<>();

        mCancel = false;

        while (!mCancel) {
            float normalizedValue = getForce();

            mValues.add(normalizedValue);
            if (mValues.size() > NUMBER_AVERAGE_VALUES) {
                mValues.remove(0);
            }

            value = getAverage(mValues);

            if (Math.abs(previousNormalizedValue - value) > sensitivity || value < minimumValue) {
                startTime = System.currentTimeMillis();
            }

            final long timePassed = System.currentTimeMillis() - startTime;

            if (timePassed >= time) {
                callback.onDone();
                return value;
            } else {
                callback.onUpdate(value, time - timePassed);
            }

            previousNormalizedValue = normalizedValue;
        }

        if (mCancel) {
            throw new CanceledException();
        }

        return 0.0f;
    }

    public void cancelCalibration() {
        mCancel = true;
    }

    private float getForce() {
        if (mFingerprintSenseTouch != null) {
            try {
                return mFingerprintSenseTouch.getForce() / (float) MAX_FORCE;
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        return 0.0f;
    }

    public boolean isSupported() {
        if (mFingerprintSenseTouch != null) {
            try {
                return mFingerprintSenseTouch.isSupported();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        return false;
    }

    public boolean finishCalibration(final float ground, final float threshold) {
        mLogger.enter("finishCalibration(ground: " + ground + ", threshold: " + threshold + ")");
        boolean status = false;
        if (mFingerprintSenseTouch != null) {
            try {
                status = mFingerprintSenseTouch.finishCalibration(Math.round(ground * MAX_FORCE),
                                                                  Math.round(threshold * MAX_FORCE));
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("finishCalibration(status: " + status + ")");
        return status;
    }

    private float getAverage(ArrayList<Float> values) {
        float value = 0.0f;
        for (Float f : values) {
            value += f;
        }
        return value /= values.size();
    }

    public boolean readConfig(SenseTouchConfig senseTouchConfig) {
        mLogger.enter("readConfig(SenseTouchConfig)");
        boolean status = false;
        if (mFingerprintSenseTouch != null) {
            try {
                com.fingerprints.extension.V1_0.SenseTouchConfig config = mFingerprintSenseTouch.readConfig();
                status = config.success;
                if (status) {
                    senseTouchConfig.set(config);
                } else {
                    mLogger.e("Fail to read config");
                }
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        } else {
            mLogger.e("Error, no sense touch extension found!");
        }
        mLogger.exit("readConfig(status: " + status + ")");
        return status;
    }
}
