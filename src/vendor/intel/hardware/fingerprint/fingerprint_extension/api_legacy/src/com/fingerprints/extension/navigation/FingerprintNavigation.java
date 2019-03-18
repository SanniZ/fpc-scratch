/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import android.os.RemoteException;

import com.fingerprints.extension.common.FingerprintExtensionBase;
import com.fingerprints.extension.util.Logger;

public class FingerprintNavigation extends FingerprintExtensionBase {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private static final String NAVIGATION = "com.fingerprints.extension.navigation.IFingerprintNavigation";
    private IFingerprintNavigation mFingerprintNavigation;

    public FingerprintNavigation() throws RemoteException {
        mLogger.enter("FingerprintNavigation");
        mFingerprintNavigation = IFingerprintNavigation.Stub.asInterface(
                getFingerprintExtension(NAVIGATION));
        if (mFingerprintNavigation == null) {
            throw new RemoteException("Could not get " + NAVIGATION);
        }
        mLogger.exit("FingerprintNavigation");
    }

    public void setNavigation(boolean enabled) {
        mLogger.enter("setNavigation");
        if (mFingerprintNavigation != null) {
            try {
                mFingerprintNavigation.setNavigation(enabled);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("setNavigation");
    }

    public NavigationConfig getNavigationConfig() {
        mLogger.enter("getNavigationConfig");
        if (mFingerprintNavigation != null) {
            try {
                return mFingerprintNavigation.getNavigationConfig();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getNavigationConfig");
        return null;
    }

    public void setNavigationConfig(NavigationConfig navigationConfig) {
        mLogger.enter("setNavigationConfig");
        if (mFingerprintNavigation != null) {
            try {
                mFingerprintNavigation.setNavigationConfig(navigationConfig);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("setNavigationConfig");
    }

    public boolean isEnabled() {
        mLogger.enter("isEnabled");
        boolean enabled = false;
        if (mFingerprintNavigation != null) {
            try {
                enabled = mFingerprintNavigation.isEnabled();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("isEnabled");
        return enabled;
    }
}
