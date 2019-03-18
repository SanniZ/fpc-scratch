/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "FingerprintSenseTouch.h"
#include "fpc_hal_ext_sense_touch_service.h"
#include <utils/Log.h>

void add_sense_touch_service(fpc_sense_touch_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintSenseTouch::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintSenseTouch* FingerprintSenseTouch::sInstance = NULL;

void FingerprintSenseTouch::instantiate(fpc_sense_touch_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintSenseTouch(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintSenseTouch");
        }
    }
}

FingerprintSenseTouch::FingerprintSenseTouch(fpc_sense_touch_t* device)
        : mDevice(device) {
}

FingerprintSenseTouch::~FingerprintSenseTouch() {
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintSenseTouch follow.
Return<uint32_t> FingerprintSenseTouch::getForce() {
    int status = -1;
    uint8_t force = 0;

    if (mDevice) {
        status = mDevice->get_force(mDevice, &force);
    }

    return (uint32_t)force;
}

Return<bool> FingerprintSenseTouch::isSupported() {
    int status = -1;
    uint8_t result = 0;

    if (mDevice) {
        status = mDevice->is_supported(mDevice, &result);
    }

    return (bool)(result == 1);
}

Return<bool> FingerprintSenseTouch::finishCalibration(uint32_t ground, uint32_t threshold) {
    ALOGE("Enter %s ground: %d, threshold: %d", __func__, ground, threshold);
    int status = -1;

    if (mDevice) {
        status = mDevice->store_calibration_data(ground, threshold);
    }
    ALOGD("Exit %s status: %d", __func__, status);
    return (bool)(status == 0);
}

Return<bool> FingerprintSenseTouch::setAuthMode(SenseTouchAuthenticationMode mode,
                                                uint32_t buttonTimeoutMs) {
    int status = -1;
    if (mDevice) {
        status = mDevice->set_auth_mode(mode != SenseTouchAuthenticationMode::NORMAL,
                                        mode == SenseTouchAuthenticationMode::ON_FORCE_RELEASE,
                                        buttonTimeoutMs);
    }

    return (bool)(status == 0);
}

Return<void> FingerprintSenseTouch::readConfig(readConfig_cb _hidl_cb) {
    ALOGD("Enter %s", __func__);
    int status = -1;
    SenseTouchConfig senseConfig;
    memset(&senseConfig, 0, sizeof(senseConfig));
    const fpc_sense_touch_config_t* config = NULL;

    if (mDevice) {
        status = mDevice->read_config(&config);
    }

    senseConfig.success = false;

    if(!status) {
        senseConfig.success = true;
        senseConfig.version = config->version;
        senseConfig.ground = config->ground;
        senseConfig.triggerThreshold = config->trigger_threshold;
        senseConfig.untriggerThreshold = config->untrigger_threshold;
        senseConfig.authTriggerOnDown = config->auth_enable_down_force;
        senseConfig.authTriggerOnUp = config->auth_enable_up_force;
        senseConfig.authButtonTimeoutMs = config->auth_button_timeout_ms;
    }

    _hidl_cb(senseConfig);

    ALOGD("Exit %s status: %d", __func__, status);

    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
