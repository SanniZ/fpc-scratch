#include "FingerprintNavigation.h"
#include "fpc_hal_ext_navigation_service.h"

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/StrongPointer.h>
#include <binder/IPCThreadState.h>

void add_navigation_service(fpc_navigation_t* device) {
    com::fingerprints::extension::V1_0::implementation::FingerprintNavigation::instantiate(device);
}

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

FingerprintNavigation* FingerprintNavigation::sInstance = NULL;

void FingerprintNavigation::instantiate(fpc_navigation_t* device) {
    if (sInstance == NULL) {
        sInstance = new FingerprintNavigation(device);
        if (sInstance->registerAsService() != android::OK) {
            ALOGE("Failed to register FingerprintNavigation");
        }
    }
}

FingerprintNavigation::FingerprintNavigation(fpc_navigation_t* device)
        : mDevice(device) {
}

FingerprintNavigation::~FingerprintNavigation() {
}

// Methods from ::com::fingerprints::extension::V1_0::IFingerprintNavigation follow.
Return<void> FingerprintNavigation::setNavigation(bool enabled) {
    if (mDevice) {
        mDevice->set_enabled(mDevice, enabled);
    }
    return Void();
}

Return<void> FingerprintNavigation::getNavigationConfig(getNavigationConfig_cb _hidl_cb) {
    if (mDevice) {
        fpc_nav_config_t config;
        mDevice->get_config(mDevice, &config);
        NavigationConfig navigationConfig;
        navigationConfig.singleClickMinTimeThreshold = config.single_click_min_time_threshold;
        navigationConfig.holdClickTimeThreshold = config.hold_click_time_threshold;
        navigationConfig.doubleClickTimeInterval = config.double_click_time_interval;
        navigationConfig.fastMoveTolerance = config.fast_move_tolerance;
        navigationConfig.slowSwipeUpThreshold = config.slow_swipe_up_threshold;
        navigationConfig.slowSwipeDownThreshold = config.slow_swipe_down_threshold;
        navigationConfig.slowSwipeLeftThreshold = config.slow_swipe_left_threshold;
        navigationConfig.slowSwipeRightThreshold = config.slow_swipe_right_threshold;
        navigationConfig.fastSwipeUpThreshold = config.fast_swipe_up_threshold;
        navigationConfig.fastSwipeDownThreshold = config.fast_swipe_down_threshold;
        navigationConfig.fastSwipeLeftThreshold = config.fast_swipe_left_threshold;
        navigationConfig.fastSwipeRightThreshold = config.fast_swipe_right_threshold;
        _hidl_cb(navigationConfig);
    }
    return Void();
}

Return<void> FingerprintNavigation::setNavigationConfig(const NavigationConfig& navigationConfig) {
    if (mDevice) {
        fpc_nav_config_t config;
        config.single_click_min_time_threshold = navigationConfig.singleClickMinTimeThreshold;
        config.hold_click_time_threshold = navigationConfig.holdClickTimeThreshold;
        config.double_click_time_interval = navigationConfig.doubleClickTimeInterval;
        config.fast_move_tolerance = navigationConfig.fastMoveTolerance;
        config.slow_swipe_up_threshold = navigationConfig.slowSwipeUpThreshold;
        config.slow_swipe_down_threshold = navigationConfig.slowSwipeDownThreshold;
        config.slow_swipe_left_threshold = navigationConfig.slowSwipeLeftThreshold;
        config.slow_swipe_right_threshold = navigationConfig.slowSwipeRightThreshold;
        config.fast_swipe_up_threshold = navigationConfig.fastSwipeUpThreshold;
        config.fast_swipe_down_threshold = navigationConfig.fastSwipeDownThreshold;
        config.fast_swipe_left_threshold = navigationConfig.fastSwipeLeftThreshold;
        config.fast_swipe_right_threshold = navigationConfig.fastSwipeRightThreshold;

        mDevice->set_config(mDevice, &config);
    }
    return Void();
}

Return<bool> FingerprintNavigation::isEnabled() {
    bool enabled = false;
    if (mDevice) {
        enabled = mDevice->get_enabled(mDevice);
    }
    return enabled;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
