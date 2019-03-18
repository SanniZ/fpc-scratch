package com.fingerprints.extension.sensetouch;

public interface CalibrationCallback {
    public abstract void onUpdate(final float value, final long remainingTime);

    public abstract void onDone();
}