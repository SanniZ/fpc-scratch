/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.recalibration;

import android.os.Handler;
import android.os.RemoteException;

import com.fingerprints.extension.common.FingerprintExtensionBase;
import com.fingerprints.extension.recalibration.IFingerprintRecalibration;
import com.fingerprints.extension.recalibration.IRecalibrationCallback;
import com.fingerprints.extension.util.Logger;

public class FingerprintRecalibration extends FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String RECALIBRATION = "com.fingerprints.extension.recalibration.IFingerprintRecalibration";
    private IFingerprintRecalibration mFingerprintRecalibration;
    private RecalibrationCallback mRecalibrationCallback;
    private Handler mHandler;

    //needs to be in sync with fpc_hal_ext_recalibration.h
    public enum Status {
        WAITING_FOR_INPUT,
        UPDATE,
        DONE;
    }

    //needs to be in sync with fpc_hal_ext_recalibration.h
    public enum Error {
        AUTHORIZATION,
        TIMEOUT,
        FAILED,
        CANCELED,
        MEMORY,
        INTERNAL;
    }

    public interface RecalibrationCallback {
        public void onStatus(Status status, boolean imageDecision, int imageQuality, int pnQuality, int progress);

        public void onError(Error error);
    }

    private IRecalibrationCallback mIRecalibrationCallback = new IRecalibrationCallback.Stub() {

        public void onStatus(int code, boolean imageDecision, int imageQuality, int pnQuality, int progress) {
            mLogger.enter("onStatus");
            mRecalibrationCallback.onStatus(Status.values()[code], imageDecision, imageQuality, pnQuality, progress);
            mLogger.exit("onStatus");
        }

        public void onError(int error) {
            mLogger.enter("onError");
            mRecalibrationCallback.onError(Error.values()[error]);
            mLogger.exit("onError");
        }
    };

    public FingerprintRecalibration() throws RemoteException {
        mLogger.enter("FingerprintRecalibration");
        mHandler = new Handler();
        mFingerprintRecalibration = IFingerprintRecalibration.Stub.asInterface(getFingerprintExtension(RECALIBRATION));
        if (mFingerprintRecalibration == null) {
            throw new RemoteException("Could not get " + RECALIBRATION);
        }
        mLogger.exit("FingerprintRecalibration");
    }

    public void recalibrate(byte[] token, RecalibrationCallback callback) {
        mLogger.enter("recalibrate");
        mRecalibrationCallback = callback;
        if (mFingerprintRecalibration != null) {
            try {
                mFingerprintRecalibration.recalibrate(token, mIRecalibrationCallback);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("recalibrate");
    }

    public long preRecalibrate() {
        mLogger.enter("pre recalibrate");
        if (mFingerprintRecalibration != null) {
            try {
                return mFingerprintRecalibration.preRecalibrate();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("pre recalibrate");
        return 0;
    }

    public void cancel() {
        mLogger.enter("cancel");
        if (mFingerprintRecalibration != null) {
            try {
                mFingerprintRecalibration.cancel();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancel");
    }
}
