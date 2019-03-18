/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/


#ifndef FPC_HIDL_H
#define FPC_HIDL_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprintClientCallback.h>
#include "fpc_tee_hal.h"

using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_array;
using ::android::sp;

class fpc_hidl : public IBiometricsFingerprint {
public:
    fpc_hidl();
    ~fpc_hidl();

    int init();

    Return<uint64_t> setNotify(const sp<IBiometricsFingerprintClientCallback>&clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid, uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId, uint32_t gid) override;

private:
    static void on_enroll_result(void* context, uint32_t fid, uint32_t gid,
                             uint32_t remaining);
    static void on_acquired(void* context, int code);
    static void on_authenticated(void* context, uint32_t fid, uint32_t gid,
                                 const uint8_t* token, uint32_t size_token);
    static void on_error(void* context, int code);
    static void on_removed(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);
    static void on_enumerate(void* context, uint32_t fid, uint32_t gid,
                       uint32_t remaining);

    sp<IBiometricsFingerprintClientCallback> callback;
    fpc_hal_compat_callback_t compat_callback;
    fpc_hal_common_t* hal;
};

#endif  // FPC_HIDL_H
