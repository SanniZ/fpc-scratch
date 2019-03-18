/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import android.os.Handler;
import android.os.RemoteException;

import com.fingerprints.extension.common.FingerprintExtensionBase;
import com.fingerprints.extension.util.Logger;

import java.util.List;

public class FingerprintSensorTest extends FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String SENSOR_TEST = "com.fingerprints.extension.sensortest.IFingerprintSensorTest";
    private IFingerprintSensorTest mFingerprintSensorTest;
    private SensorTestCallback mSensorTestCallback;
    private CaptureCallback mCaptureCallback;
    private Handler mHandler;

    private ISensorTestCallback mISensorTestCallback =
            new ISensorTestCallback.Stub() {
                public void onResult(final SensorTestResult result) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mSensorTestCallback != null) {
                                mSensorTestCallback.onResult(result);
                            }
                        }
                    });
                }
            };

    public interface SensorTestCallback {
        public void onResult(SensorTestResult result);
    }

    private ICaptureCallback mICaptureCallback =
            new ICaptureCallback.Stub() {
                public void onAcquired(final int acquiredInfo) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mCaptureCallback != null) {
                                mCaptureCallback.onAcquired(acquiredInfo);
                            }
                        }
                    });
                }

                public void onError(final int error) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mCaptureCallback != null) {
                                mCaptureCallback.onError(error);
                            }
                        }
                    });
                }
            };

    public interface CaptureCallback {
        public void onAcquired(int acquiredInfo);

        public void onError(int error);
    }

    public FingerprintSensorTest() throws RemoteException {
        mLogger.enter("FingerprintSensorTest");
        mHandler = new Handler();
        mFingerprintSensorTest = IFingerprintSensorTest.Stub.asInterface(
                getFingerprintExtension(SENSOR_TEST));
        if (mFingerprintSensorTest == null) {
            throw new RemoteException("Could not get " + SENSOR_TEST);
        }
        mLogger.exit("FingerprintSensorTest");
    }

    public SensorInfo getSensorInfo() {
        mLogger.enter("getSensorInfo");
        SensorInfo sensorInfo = null;
        if (mFingerprintSensorTest != null) {
            try {
                sensorInfo = mFingerprintSensorTest.getSensorInfo();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getSensorInfo");
        return sensorInfo;
    }

    public List<SensorTest> getSensorTests() {
        mLogger.enter("getSensorTests");
        List<SensorTest> tests = null;
        if (mFingerprintSensorTest != null) {
            try {
                tests = mFingerprintSensorTest.getSensorTests();
            } catch (RemoteException e) {
                mLogger.e("getSensorTests: ", e);
            }
        }
        mLogger.exit("getSensorTests");
        return tests;
    }

    public void runSensorTest(SensorTestCallback callback, SensorTest test, SensorTestInput input) {
        mLogger.enter("runSensorTest");
        mSensorTestCallback = callback;
        if (mFingerprintSensorTest != null) {
            try {
                mFingerprintSensorTest.runSensorTest(mISensorTestCallback, test, input);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("runSensorTest");
    }

    public void cancelSensorTest() {
        mLogger.enter("cancelSensorTest");
        if (mFingerprintSensorTest != null) {
            try {
                mFingerprintSensorTest.cancelSensorTest();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancelSensorTest");
    }

    public void capture(CaptureCallback callback, boolean waitForFinger, boolean uncalibrated) {
        mLogger.enter("capture");
        mCaptureCallback = callback;
        if (mFingerprintSensorTest != null) {
            try {
                mFingerprintSensorTest.capture(mICaptureCallback, waitForFinger, uncalibrated);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("capture");
    }

    public void cancelCapture() {
        mLogger.enter("cancelCapture");
        if (mFingerprintSensorTest != null) {
            try {
                mFingerprintSensorTest.cancelCapture();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancelCapture");
    }
}
