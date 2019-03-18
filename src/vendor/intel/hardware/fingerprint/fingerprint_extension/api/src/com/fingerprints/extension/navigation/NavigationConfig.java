/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.navigation;

import com.fingerprints.extension.util.Logger;

public class NavigationConfig {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int singleClickMinTimeThreshold;
    public int holdClickTimeThreshold;
    public int doubleClickTimeInterval;
    public int fastMoveTolerance;
    public int slowSwipeUpThreshold;
    public int slowSwipeDownThreshold;
    public int slowSwipeLeftThreshold;
    public int slowSwipeRightThreshold;
    public int fastSwipeUpThreshold;
    public int fastSwipeDownThreshold;
    public int fastSwipeLeftThreshold;
    public int fastSwipeRightThreshold;

    public NavigationConfig() {
        singleClickMinTimeThreshold = 0;
        holdClickTimeThreshold      = 0;
        doubleClickTimeInterval     = 0;
        fastMoveTolerance           = 0;
        slowSwipeUpThreshold        = 0;
        slowSwipeDownThreshold      = 0;
        slowSwipeLeftThreshold      = 0;
        slowSwipeRightThreshold     = 0;
        fastSwipeUpThreshold        = 0;
        fastSwipeDownThreshold      = 0;
        fastSwipeLeftThreshold      = 0;
        fastSwipeRightThreshold     = 0;
    }

    public NavigationConfig(NavigationConfig objToCopy) {
        this.singleClickMinTimeThreshold = objToCopy.singleClickMinTimeThreshold;
        this.holdClickTimeThreshold      = objToCopy.holdClickTimeThreshold;
        this.doubleClickTimeInterval     = objToCopy.doubleClickTimeInterval;
        this.fastMoveTolerance           = objToCopy.fastMoveTolerance;
        this.slowSwipeUpThreshold        = objToCopy.slowSwipeUpThreshold;
        this.slowSwipeDownThreshold      = objToCopy.slowSwipeDownThreshold;
        this.slowSwipeLeftThreshold      = objToCopy.slowSwipeLeftThreshold;
        this.slowSwipeRightThreshold     = objToCopy.slowSwipeRightThreshold;
        this.fastSwipeUpThreshold        = objToCopy.fastSwipeUpThreshold;
        this.fastSwipeDownThreshold      = objToCopy.fastSwipeDownThreshold;
        this.fastSwipeLeftThreshold      = objToCopy.fastSwipeLeftThreshold;
        this.fastSwipeRightThreshold     = objToCopy.fastSwipeRightThreshold;
    }

    NavigationConfig(com.fingerprints.extension.V1_0.NavigationConfig config) {
        this.singleClickMinTimeThreshold = config.singleClickMinTimeThreshold;
        this.holdClickTimeThreshold      = config.holdClickTimeThreshold;
        this.doubleClickTimeInterval     = config.doubleClickTimeInterval;
        this.fastMoveTolerance           = config.fastMoveTolerance;
        this.slowSwipeUpThreshold        = config.slowSwipeUpThreshold;
        this.slowSwipeDownThreshold      = config.slowSwipeDownThreshold;
        this.slowSwipeLeftThreshold      = config.slowSwipeLeftThreshold;
        this.slowSwipeRightThreshold     = config.slowSwipeRightThreshold;
        this.fastSwipeUpThreshold        = config.fastSwipeUpThreshold;
        this.fastSwipeDownThreshold      = config.fastSwipeDownThreshold;
        this.fastSwipeLeftThreshold      = config.fastSwipeLeftThreshold;
        this.fastSwipeRightThreshold     = config.fastSwipeRightThreshold;
    }

    com.fingerprints.extension.V1_0.NavigationConfig getHidlConfig() {
        com.fingerprints.extension.V1_0.NavigationConfig config =
                new com.fingerprints.extension.V1_0.NavigationConfig();

        config.singleClickMinTimeThreshold = this.singleClickMinTimeThreshold;
        config.holdClickTimeThreshold      = this.holdClickTimeThreshold;
        config.doubleClickTimeInterval     = this.doubleClickTimeInterval;
        config.fastMoveTolerance           = this.fastMoveTolerance;
        config.slowSwipeUpThreshold        = this.slowSwipeUpThreshold;
        config.slowSwipeDownThreshold      = this.slowSwipeDownThreshold;
        config.slowSwipeLeftThreshold      = this.slowSwipeLeftThreshold;
        config.slowSwipeRightThreshold     = this.slowSwipeRightThreshold;
        config.fastSwipeUpThreshold        = this.fastSwipeUpThreshold;
        config.fastSwipeDownThreshold      = this.fastSwipeDownThreshold;
        config.fastSwipeLeftThreshold      = this.fastSwipeLeftThreshold;
        config.fastSwipeRightThreshold     = this.fastSwipeRightThreshold;
        return config;
    }

    public void print() {
        mLogger.d("singleClickMinTimeThreshold: " + singleClickMinTimeThreshold +
                  " holdClickTimeThreshold: "     + holdClickTimeThreshold +
                  " doubleClickTimeInterval: "    + doubleClickTimeInterval +
                  " fastMoveTolerance: "          + fastMoveTolerance +
                  " slowSwipeUpThreshold: "       + slowSwipeUpThreshold +
                  " slowSwipeDownThreshold: "     + slowSwipeDownThreshold +
                  " slowSwipeLeftThreshold: "     + slowSwipeLeftThreshold +
                  " slowSwipeRightThreshold: "    + slowSwipeRightThreshold +
                  " fastSwipeUpThreshold: "       + fastSwipeUpThreshold +
                  " fastSwipeDownThreshold: "     + fastSwipeDownThreshold +
                  " fastSwipeLeftThreshold: "     + fastSwipeLeftThreshold +
                  " fastSwipeRightThreshold: "    + fastSwipeRightThreshold);
    }
}
