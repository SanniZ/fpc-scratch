#
# Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.biometrics.fpcfingerprint@2.1-service
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.biometrics.fpcfingerprint@2.1-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := fpc
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := service.cpp fpc_hidl.cpp
LOCAL_CFLAGS := -DLOG_TAG='"fpc_hidl"'

ifeq ($(filter 6.% 7.%, $(PLATFORM_VERSION)),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_C_INCLUDES += \
	frameworks/native/libs/binder/include/

LOCAL_STATIC_LIBRARIES := fpc_hal_common fpc_tac libcutils fpc_hal_extension

LOCAL_SHARED_LIBRARIES := liblog libhidlbase libhidltransport \
                          libutils android.hardware.biometrics.fingerprint@2.1 \
                          com.fingerprints.extension@1.0


ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
else ifeq ($(FPC_TEE_RUNTIME), TOS)
LOCAL_SHARED_LIBRARIES += libtrusty
endif

include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))
