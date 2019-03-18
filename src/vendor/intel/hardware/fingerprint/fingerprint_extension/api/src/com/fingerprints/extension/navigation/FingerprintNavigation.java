/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import android.os.RemoteException;

import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.V1_0.IFingerprintNavigation;
import com.fingerprints.extension.navigation.NavigationConfig;

public class FingerprintNavigation {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private IFingerprintNavigation mFingerprintNavigation;

    public FingerprintNavigation() throws RemoteException {
        mLogger.enter("FingerprintNavigation");

        mFingerprintNavigation = IFingerprintNavigation.getService();
        if (mFingerprintNavigation == null) {
            throw new RemoteException("Could not get IFingerprintNavigation service");
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
        NavigationConfig navigationConfig = null;
        if (mFingerprintNavigation != null) {
            try {
                navigationConfig = new NavigationConfig(mFingerprintNavigation.getNavigationConfig());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getNavigationConfig");
        return navigationConfig;
    }

    public void setNavigationConfig(NavigationConfig navigationConfig) {
        mLogger.enter("setNavigationConfig");
        if (mFingerprintNavigation != null) {
            try {
                mFingerprintNavigation.setNavigationConfig(navigationConfig.getHidlConfig());
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
