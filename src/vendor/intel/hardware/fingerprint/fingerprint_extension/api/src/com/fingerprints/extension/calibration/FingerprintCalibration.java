/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.calibration;

import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;

import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.V1_0.IFingerprintCalibration;
import com.fingerprints.extension.V1_0.ICalibrationCallback;

public class FingerprintCalibration {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private IFingerprintCalibration mFingerprintCalibration;
    private CalibrationCallback mCalibrationCallback;
    private Handler mHandler;

    //needs to be in sync with fpc_hal_ext_calibration.h
    public enum Status {
        WAITING_FOR_INPUT,
        STABILIZE,
        START,
        RETRY,
        DONE;
    }

    public interface CalibrationCallback {
        public void onStatus(Status status);

        public void onError(int error);
    }

    private ICalibrationCallback mICalibrationCallback = new ICalibrationCallback.Stub() {
        public void onStatus(int code) {
            mLogger.enter("onStatus");
            mCalibrationCallback.onStatus(Status.values()[code]);
            mLogger.exit("onStatus");
        }

        public void onError(int error) {
            mLogger.enter("onError");
            mCalibrationCallback.onError(error);
            mLogger.exit("onError");
        }
    };

    public FingerprintCalibration() throws RemoteException {
        mLogger.enter("FingerprintCalibration");
        mHandler = new Handler();

        mFingerprintCalibration = IFingerprintCalibration.getService();
        if (mFingerprintCalibration == null) {
            throw new RemoteException("Could not get IFingerprintCalibration service");
        }
        mLogger.exit("FingerprintCalibration");
    }

    public void calibrate(CalibrationCallback callback) {
        mLogger.enter("calibrate");
        mCalibrationCallback = callback;
        if (mFingerprintCalibration != null) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Looper.prepare();
                    try {
                        mFingerprintCalibration.calibrate(mICalibrationCallback);
                    } catch (RemoteException e) {
                        mLogger.e("RemoteException: ", e);
                    }
                }
            }).start();
        }
        mLogger.exit("calibrate");
    }
}
