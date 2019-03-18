/**
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.navigation;

import com.fingerprints.extension.navigation.NavigationConfig;

/** {@hide} */
interface IFingerprintNavigation {
    void setNavigation(boolean enabled);
    NavigationConfig getNavigationConfig();
    void setNavigationConfig(in NavigationConfig navigationConfig);
    boolean isEnabled();
}
