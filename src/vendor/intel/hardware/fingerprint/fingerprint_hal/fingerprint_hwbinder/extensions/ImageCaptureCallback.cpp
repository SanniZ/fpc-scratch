#include "ImageCaptureCallback.h"

namespace com {
namespace fingerprints {
namespace extension {
namespace V1_0 {
namespace implementation {

// Methods from IImageCaptureCallback follow.
Return<void> ImageCaptureCallback::onImage(const ImageCaptureData& imageCaptureData) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IImageCaptureCallback* HIDL_FETCH_IImageCaptureCallback(const char* /* name */) {
//    return new ImageCaptureCallback();
//}

}  // namespace implementation
}  // namespace V1_0
}  // namespace extension
}  // namespace fingerprints
}  // namespace com
