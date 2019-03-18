/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#include <stdint.h>

#include <vector>


#include "fpc_hidl.h"
#include "fpc_tee_hal.h"
#include "fpc_log.h"

fpc_hidl::fpc_hidl() : hal(nullptr)
{
    compat_callback.on_acquired = on_acquired;
    compat_callback.on_authenticated = on_authenticated;
    compat_callback.on_enroll_result = on_enroll_result;
    compat_callback.on_enumerate = on_enumerate;
    compat_callback.on_error = on_error;
    compat_callback.on_removed = on_removed;
}

fpc_hidl::~fpc_hidl()
{
    fpc_hal_close(hal);
}

int fpc_hidl::init()
{
    return fpc_hal_open(&hal, &compat_callback, this);
}

Return<uint64_t> fpc_hidl::setNotify(const sp<IBiometricsFingerprintClientCallback>& clientCallback)
{
    callback = clientCallback;
    return reinterpret_cast<uint64_t>(this);
}

Return<uint64_t> fpc_hidl::preEnroll()
{
    return fpc_pre_enroll(hal);
}

Return<RequestStatus> fpc_hidl::enroll(const hidl_array<uint8_t, 69>& hat,
                                       uint32_t gid, uint32_t timeoutSec)
{
    if (fpc_enroll(hal, static_cast<const uint8_t*>(hat.data()), 69, gid, timeoutSec)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::postEnroll()
{
    if (fpc_post_enroll(hal)) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<uint64_t> fpc_hidl::getAuthenticatorId()
{
    return fpc_get_authenticator_id(hal);
}

Return<RequestStatus> fpc_hidl::cancel()
{
    fpc_cancel(hal);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::enumerate()
{
    fpc_enumerate(hal);
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::remove(uint32_t gid, uint32_t fid)
{
    if (fpc_remove(hal, gid, fid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::setActiveGroup(uint32_t gid, const hidl_string& storePath)
{
    if (storePath.size() >= PATH_MAX || storePath.size() <= 0) {
        ALOGE("Bad path length: %zd", storePath.size());
        return RequestStatus::SYS_EINVAL;
    }
    if (access(storePath.c_str(), W_OK)) {
        return RequestStatus::SYS_EINVAL;
    }

    if (fpc_set_active_group(hal, gid, storePath.c_str())) {
        return RequestStatus::SYS_UNKNOWN;
    }
    return RequestStatus::SYS_OK;
}

Return<RequestStatus> fpc_hidl::authenticate(uint64_t operationId, uint32_t gid)
{
    if (fpc_authenticate(hal, operationId, gid)) {
        return RequestStatus::SYS_EINVAL;
    }
    return RequestStatus::SYS_OK;
}

void fpc_hidl::on_enroll_result(void* context, uint32_t fid, uint32_t gid, uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    ALOGD("onEnrollResult(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->callback->onEnrollResult(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnrollResult callback");
    }
}

void fpc_hidl::on_acquired(void* context, int code)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintAcquiredInfo acquired_info = static_cast<FingerprintAcquiredInfo>(code);

    if (code >= HAL_COMPAT_VENDOR_BASE) {
        vendor_code = code - HAL_COMPAT_VENDOR_BASE;
        acquired_info = FingerprintAcquiredInfo::ACQUIRED_VENDOR;
    }
    ALOGD("onAcquired(code=%d, vendor=%d)", acquired_info, vendor_code);
    if (!me->callback->onAcquired(reinterpret_cast<uint64_t>(context),
                acquired_info, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onAcquired callback");
    }
}

void fpc_hidl::on_authenticated(void* context, uint32_t fid, uint32_t gid,
                                const uint8_t* token, uint32_t size_token)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    const std::vector<uint8_t> hidl_token(token, token + size_token);
    ALOGD("onAuthenticated(fid=%d, gid=%d)", fid, gid);
    if (!me->callback->onAuthenticated(reinterpret_cast<uint64_t>(context),
                fid, gid, hidl_vec<uint8_t>(hidl_token)).isOk()) {
        ALOGE("failed to invoke fingerprint onAuthenticated callback");
    }
}

void fpc_hidl::on_error(void* context, int code)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    int32_t vendor_code = 0;
    FingerprintError error = static_cast<FingerprintError>(code);

    if (code >= HAL_COMPAT_VENDOR_BASE) {
        vendor_code = code - HAL_COMPAT_VENDOR_BASE;
        error = FingerprintError::ERROR_VENDOR;
    }
    ALOGD("onError(error=%d, vendor=%d)", code, vendor_code);
    if (!me->callback->onError(reinterpret_cast<uint64_t>(context),
                error, vendor_code).isOk()) {
        ALOGE("failed to invoke fingerprint onError callback");
    }
}

void fpc_hidl::on_removed(void* context, uint32_t fid, uint32_t gid,
                   uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    ALOGD("onRemoved(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->callback->onRemoved(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onRemoved callback");
    }
}

void fpc_hidl::on_enumerate(void* context, uint32_t fid,
                                       uint32_t gid, uint32_t remaining)
{
    fpc_hidl* me = static_cast<fpc_hidl*>(context);
    ALOGD("onEnumerate(fid=%d, gid=%d, rem=%d)", fid, gid, remaining);
    if (!me->callback->onEnumerate(reinterpret_cast<uint64_t>(context),
                fid, gid, remaining).isOk()) {
        ALOGE("failed to invoke fingerprint onEnumerate callback");
    }
}
