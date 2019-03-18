/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FINGERPRINT_NAVIGATION_H_
#define FINGERPRINT_NAVIGATION_H_

#include "IFingerprintNavigation.h"
#include "fpc_hal_ext_navigation.h"

namespace android {

class FingerprintNavigation : public BnFingerprintNavigation {
     public:
        static void instantiate(fpc_navigation_t* device);

        // These reflect binder methods.
        virtual void setNavigation(bool enabled);
        virtual void getNavigationConfig(fpc_nav_config_t* config);
        virtual void setNavigationConfig(const fpc_nav_config_t* config);
        virtual void isEnabled(bool* enabled);

     private:
        FingerprintNavigation(fpc_navigation_t* device);
        virtual ~FingerprintNavigation();

        static FingerprintNavigation* sInstance;
        fpc_navigation_t* mDevice;
};

}  // namespace android

#endif // FINGERPRINT_NAVIGATION_H_
