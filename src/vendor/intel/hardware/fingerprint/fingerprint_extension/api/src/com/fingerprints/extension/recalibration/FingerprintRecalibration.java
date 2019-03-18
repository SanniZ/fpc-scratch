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

import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.V1_0.IFingerprintRecalibration;
import com.fingerprints.extension.V1_0.IRecalibrationCallback;

import java.util.ArrayList;
import java.util.Arrays;

public class FingerprintRecalibration {
    private Logger mLogger = new Logger(getClass().getSimpleName());
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
        mFingerprintRecalibration = IFingerprintRecalibration.getService();
        if (mFingerprintRecalibration == null) {
            throw new RemoteException("Could not get IFingerprintRecalibration service");
        }
        mLogger.exit("FingerprintRecalibration");
    }

    public void recalibrate(byte[] token, RecalibrationCallback callback) {
        mLogger.enter("recalibrate");
        mRecalibrationCallback = callback;
        if (mFingerprintRecalibration != null) {
            try {
                ArrayList<Byte> arrayList = new ArrayList<Byte>();
                for (byte tok : token) {
                    arrayList.add(tok);
                }
                mFingerprintRecalibration.recalibrate(arrayList,
                        mIRecalibrationCallback);
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
