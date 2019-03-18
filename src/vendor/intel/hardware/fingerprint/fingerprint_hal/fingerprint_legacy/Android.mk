#
# Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# TODO: Update LOCAL_MODULE to match the correct project setup
ifeq ($(TARGET_DEVICE),evb6795_64_tee)
LOCAL_MODULE := fingerprint.mt6795
else ifeq ($(PRODUCT_MODEL),evb6797_64_teei)
LOCAL_MODULE := fingerprint.mt6797
else
LOCAL_MODULE := fingerprint.$(TARGET_DEVICE)
LOCAL_MULTILIB := first
endif
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := fpc

ifeq ($(filter 6.% 7.%, $(PLATFORM_VERSION)),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CFLAGS := -std=c99 -Wall -Werror\
    -DLOG_TAG='"fpc_fingerprint_hal"' \
    -DFPC_TZ_APP_NAME="\"fpctzappfingerprint\""

LOCAL_SRC_FILES := fpc_legacy_hal.c

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS    += -DFPC_CONFIG_HW_AUTH
endif


LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/interface/ \
                    hardware/libhardware/include \

LOCAL_SHARED_LIBRARIES := liblog \
                          libbinder \
                          libutils

LOCAL_STATIC_LIBRARIES += fpc_hal_common fpc_tac libcutils fpc_hal_extension_legacy

ifeq ($(FPC_TEE_RUNTIME), TBASE)
LOCAL_SHARED_LIBRARIES += libMcClient
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
LOCAL_SHARED_LIBRARIES += lib_fpc_ta_shared
else ifeq ($(FPC_TEE_RUNTIME), TOS)
LOCAL_SHARED_LIBRARIES += libtrusty
endif

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
