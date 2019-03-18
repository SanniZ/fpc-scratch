/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef IFINGERPRINT_NAVIGATION_H_
#define IFINGERPRINT_NAVIGATION_H_

#include <inttypes.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "fpc_hal_ext_navigation.h"

namespace android {

class IFingerprintNavigation : public IInterface {
     public:
        // must be kept in sync with IFingerprintNavigation.aidl
        enum {
            SET_NAVIGATION = IBinder::FIRST_CALL_TRANSACTION,
            GET_NAVIGATION_CONFIG,
            SET_NAVIGATION_CONFIG,
            IS_ENABLED,
        };

        virtual const android::String16& getInterfaceDescriptor() const;

        // Binder interface methods
        virtual void setNavigation(bool enabled) = 0;
        virtual void getNavigationConfig(fpc_nav_config_t* config) = 0;
        virtual void setNavigationConfig(const fpc_nav_config_t* config) = 0;
        virtual void isEnabled(bool* enabled) = 0;

        // DECLARE_META_INTERFACE - C++ client interface not needed
        static const android::String16 descriptor;
};

// ----------------------------------------------------------------------------

class BnFingerprintNavigation : public BnInterface<IFingerprintNavigation> {
     public:
        virtual status_t onTransact(uint32_t code, const Parcel& data,
                                    Parcel* reply, uint32_t flags = 0);
     private:
        bool checkPermission(const String16& permission);
};

}  // namespace android

#endif // IFINGERPRINT_NAVIGATION_H_

