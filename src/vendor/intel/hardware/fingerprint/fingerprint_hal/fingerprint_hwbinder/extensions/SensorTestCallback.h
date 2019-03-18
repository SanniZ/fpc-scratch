#ifndef COM_FINGERPRINTS_EXTENSION_V1_0_SENSORTESTCALLBACK_H
#define COM_FINGERPRINTS_EXTENSION_V1_0_SENSORTESTCALLBACK_H

#include <com/fingerprints/extension/1.0/ISensorTestCallback.h>
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

struct SensorTestCallback : public ISensorTestCallback {
    // Methods from ISensorTestCallback follow.
    Return<void> onResult(const SensorTestResult& result) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ISensorTestCallback* HIDL_FETCH_ISensorTestCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com

#endif  // COM_FINGERPRINTS_EXTENSION_V1_0_SENSORTESTCALLBACK_H
