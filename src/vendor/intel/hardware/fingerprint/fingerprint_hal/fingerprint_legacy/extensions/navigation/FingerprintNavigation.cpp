/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <binder/IServiceManager.h>
#include "fpc_hal_ext_navigation_service.h"
#include "FingerprintNavigation.h"

void add_navigation_service(fpc_navigation_t* device) {
    android::FingerprintNavigation::instantiate(device);
}

namespace android {

FingerprintNavigation* FingerprintNavigation::sInstance = NULL;

void FingerprintNavigation::instantiate(fpc_navigation_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintNavigation(device);
    }
    defaultServiceManager()->addService(descriptor, sInstance);
}

FingerprintNavigation::FingerprintNavigation(fpc_navigation_t* device) {
    mDevice = device;
}

FingerprintNavigation::~FingerprintNavigation() {
}

void FingerprintNavigation::setNavigation(bool enabled) {
    if (mDevice) {
        mDevice->set_enabled(mDevice, enabled);
    }
}

void FingerprintNavigation::getNavigationConfig(fpc_nav_config_t* config) {
    if (mDevice) {
        mDevice->get_config(mDevice, config);
    }
}

void FingerprintNavigation::setNavigationConfig(const fpc_nav_config_t* config) {
    if (mDevice) {
        mDevice->set_config(mDevice, config);
    }
}

void FingerprintNavigation::isEnabled(bool* enabled) {
    if (mDevice) {
        *enabled = mDevice->get_enabled(mDevice);
    }
}

}
