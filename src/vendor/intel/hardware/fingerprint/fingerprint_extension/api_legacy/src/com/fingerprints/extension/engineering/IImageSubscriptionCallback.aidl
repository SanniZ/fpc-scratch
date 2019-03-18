/**
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.engineering;

/** {@hide} */
interface IImageSubscriptionCallback {
    void onImage(int mode, int capture_result, int identify_result, int template_update_result,
                  int enroll_result, int cac_result, int userId, int remaining_samples, int coverage,
                  int quality, in byte[] rawImage, in byte[] enhancedImage);
}
