/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/types.h>
#include <utils/StrongPointer.h>
#include <binder/IPCThreadState.h>
#include "fpc_hidl.h"

using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::sp;

int main()
{
    ALOGD("fpc fingerprint hwbinder service starting");

    sp<fpc_hidl> hal = new fpc_hidl();

    if (hal == nullptr) {
        return -1;
    }

    if (hal->init()) {
        return -1;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);
    hal->registerAsService("default");
    joinRpcThreadpool();
    return 0;
}
