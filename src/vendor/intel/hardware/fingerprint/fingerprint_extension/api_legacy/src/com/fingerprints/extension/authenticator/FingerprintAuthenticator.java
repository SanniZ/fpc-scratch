/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.authenticator;

import android.os.Handler;
import android.os.RemoteException;

import com.fingerprints.extension.common.FingerprintExtensionBase;
import com.fingerprints.extension.util.Logger;

public class FingerprintAuthenticator extends FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String AUTHENTICATION = "com.fingerprints.extension.authenticator.IFingerprintAuthenticator";
    private IFingerprintAuthenticator mFingerprintAuthenticator;
    private VerifyUserCallback mVerifyUserCallback;
    private Handler mHandler;

    public interface VerifyUserCallback {
        public void onResult(int result, long userId, long userEntityId, byte[] encapsulatedResult);

        public void onHelp(int helpCode);
    }

    private IVerifyUserCallback mIVerifyUserCallback =
            new IVerifyUserCallback.Stub() {
                public void onResult(final int result, final long userId, final long userEntityId,
                                     final byte[] encapsulatedResult) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mVerifyUserCallback != null) {
                                mVerifyUserCallback.onResult(result, userId, userEntityId,
                                        encapsulatedResult);
                            }
                        }
                    });
                }

                public void onHelp(final int helpCode) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mVerifyUserCallback != null) {
                                mVerifyUserCallback.onHelp(helpCode);
                            }
                        }
                    });
                }
            };

    public FingerprintAuthenticator() throws RemoteException {
        mLogger.enter("FingerprintAuthenticator");
        mHandler = new Handler();
        mFingerprintAuthenticator = IFingerprintAuthenticator.Stub.asInterface(
                getFingerprintExtension(AUTHENTICATION));
        if (mFingerprintAuthenticator == null) {
            throw new RemoteException("Could not get " + AUTHENTICATION);
        }
        mLogger.exit("FingerprintAuthenticator");
    }

    public int verifyUser(VerifyUserCallback callback, byte[] nonce, String dstAppName) {
        mLogger.enter("verifyUser");
        mVerifyUserCallback = callback;
        int status = 0;
        if (mFingerprintAuthenticator != null) {
            try {
                status = mFingerprintAuthenticator.verifyUser(mIVerifyUserCallback, nonce,
                        dstAppName.getBytes());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("verifyUser");
        return status;
    }

    public boolean isUserValid(long userId) {
        mLogger.enter("isUserValid");
        boolean result = false;
        if (mFingerprintAuthenticator != null) {
            try {
                result = (mFingerprintAuthenticator.isUserValid(userId) != 0);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("isUserValid");
        return result;
    }

    public void cancel() {
        mLogger.enter("cancel");
        if (mFingerprintAuthenticator != null) {
            try {
                mFingerprintAuthenticator.cancel();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancel");
    }
}
