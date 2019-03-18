/**
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

package com.fingerprints.extension.authenticator;

import com.fingerprints.extension.authenticator.IVerifyUserCallback;

/** {@hide} */
interface IFingerprintAuthenticator {
    int verifyUser(IVerifyUserCallback callback, in byte[] nonce, in byte[] dstAppName);
    int isUserValid(long userId);
    void cancel();
}
