/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "FingerprintSenseTouch.h"

#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include "fpc_hal_ext_sense_touch_service.h"

void add_sense_touch_service(fpc_sense_touch_t* device) {
    android::FingerprintSenseTouch::instantiate(device);
}

namespace android {

FingerprintSenseTouch* FingerprintSenseTouch::sInstance = NULL;

void FingerprintSenseTouch::instantiate(fpc_sense_touch_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintSenseTouch(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintSenseTouch::FingerprintSenseTouch(fpc_sense_touch_t* device) {
    mDevice = device;
}

FingerprintSenseTouch::~FingerprintSenseTouch() {
}

void FingerprintSenseTouch::binderDied(const wp<IBinder>& who) {
    (void) who;
}

int FingerprintSenseTouch::getForce(uint8_t* force) {
    int status = -1;

    if (mDevice) {
        status = mDevice->get_force(mDevice, force);
    }

    return status;
}

int FingerprintSenseTouch::isSupported(uint8_t* result) {
    int status = -1;

    if (mDevice) {
        status = mDevice->is_supported(mDevice, result);
    }

    return status;
}

int FingerprintSenseTouch::storeCalibrationData(uint8_t ground, uint8_t threshold) {
    ALOGE("Enter %s ground: %d, threshold: %d", __func__, ground, threshold);
    int status = -1;

    if (mDevice) {
        status = mDevice->store_calibration_data(ground, threshold);
    }
    ALOGD("Exit %s status: %d", __func__, status);
    return status;
}

int FingerprintSenseTouch::setAuthMode(SenseTouchAuthenticationMode mode,
                                       uint32_t button_timeout_ms){
    int status = -1;
    if (mDevice) {
        status = mDevice->set_auth_mode(mode != SenseTouchAuthenticationMode::AUTH_MODE_NORMAL,
                                        mode == SenseTouchAuthenticationMode::AUTH_MODE_ON_FORCE_RELEASE,
                                        button_timeout_ms);
    }

    return status;
}

int FingerprintSenseTouch::readConfig(const fpc_sense_touch_config_t** senseTouchConfig) {
    ALOGD("Enter %s", __func__);
    int status = -1;

    if (mDevice) {
        status = mDevice->read_config(senseTouchConfig);
    }
    ALOGD("Exit %s status: %d", __func__, status);
    return status;
}
} /* namespace end */
