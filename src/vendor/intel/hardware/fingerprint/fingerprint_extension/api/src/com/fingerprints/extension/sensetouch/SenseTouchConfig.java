/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensetouch;

import com.fingerprints.extension.util.Logger;

public class SenseTouchConfig {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    public int version;
    public int ground;
    public int triggerThreshold;
    public int untriggerThreshold;
    public boolean authTriggerOnDown;
    public boolean authTriggerOnUp;
    public int authButtonTimeout;

    public SenseTouchConfig() {
        mLogger.enter("SenseTouchConfig()");
        version            = 0;
        ground             = 0;
        triggerThreshold   = 0;
        untriggerThreshold = 0;
        authTriggerOnDown  = false;
        authTriggerOnUp    = false;
        authButtonTimeout  = 0;
        mLogger.exit("SenseTouchConfig()");
    }

    public SenseTouchConfig(SenseTouchConfig objToCopy) {
        mLogger.enter("SenseTouchConfig(SenseTouchConfig)");
        this.version            = objToCopy.version;
        this.ground             = objToCopy.ground;
        this.triggerThreshold   = objToCopy.triggerThreshold;
        this.untriggerThreshold = objToCopy.untriggerThreshold;
        this.authTriggerOnDown  = objToCopy.authTriggerOnDown;
        this.authTriggerOnUp    = objToCopy.authTriggerOnUp;
        this.authButtonTimeout  = objToCopy.authButtonTimeout;
        mLogger.exit("SenseTouchConfig()");
    }

    public void print() {
        mLogger.enter("print()");
        mLogger.d("version: "             + version +
                  " ground: "             + ground +
                  " triggerThreshold: "   + triggerThreshold +
                  " untriggerThreshold: " + untriggerThreshold +
                  " authTriggerOnDown: "  + authTriggerOnDown +
                  " authTriggerOnUp: "    + authTriggerOnUp +
                  " authButtonTimeout: "  + authButtonTimeout);
        mLogger.exit("print()");
    }

    void set(com.fingerprints.extension.V1_0.SenseTouchConfig config) {
        this.version            = config.version;
        this.ground             = config.ground;
        this.triggerThreshold   = config.triggerThreshold;
        this.untriggerThreshold = config.untriggerThreshold;
        this.authTriggerOnDown  = config.authTriggerOnDown;
        this.authTriggerOnUp    = config.authTriggerOnUp;
        this.authButtonTimeout  = config.authButtonTimeoutMs;
    }
}
