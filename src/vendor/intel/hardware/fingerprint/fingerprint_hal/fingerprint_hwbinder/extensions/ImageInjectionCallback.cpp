#include "ImageInjectionCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IImageInjectionCallback follow.
Return<void> ImageInjectionCallback::onInject(onInject_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> ImageInjectionCallback::onCancel() {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IImageInjectionCallback* HIDL_FETCH_IImageInjectionCallback(const char* /* name */) {
//    return new ImageInjectionCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
