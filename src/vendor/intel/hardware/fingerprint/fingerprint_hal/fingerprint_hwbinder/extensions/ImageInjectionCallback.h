#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_IMAGEINJECTIONCALLBACK_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_IMAGEINJECTIONCALLBACK_H

#include <com/fingerprints/extension/1.0/IImageInjectionCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ImageInjectionCallback : public IImageInjectionCallback {
    // Methods from IImageInjectionCallback follow.
    Return<void> onInject(onInject_cb _hidl_cb) override;
    Return<void> onCancel() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IImageInjectionCallback* HIDL_FETCH_IImageInjectionCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_IMAGEINJECTIONCALLBACK_H
