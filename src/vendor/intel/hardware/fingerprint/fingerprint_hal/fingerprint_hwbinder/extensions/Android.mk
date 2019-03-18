#
# Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fpc_hal_extension

ifeq ($(filter 6.% 7.%,$(PLATFORM_VERSION)),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CFLAGS := -Wall -Werror \
    -DLOG_TAG='"fpc_hal_extension"' \

LOCAL_SRC_FILES  += authenticator/FingerprintAuthenticator.cpp \
                    calibration/FingerprintCalibration.cpp \
                    engineering/FingerprintEngineering.cpp \
                    navigation/FingerprintNavigation.cpp \
                    recalibration/FingerprintRecalibration.cpp \
                    sensetouch/FingerprintSenseTouch.cpp \
                    sensortest/FingerprintSensorTest.cpp

COMMON_INCLUDE_PATH := $(LOCAL_PATH)/../../fingerprint_common

LOCAL_C_INCLUDES += $(COMMON_INCLUDE_PATH)/ \
                    $(COMMON_INCLUDE_PATH)/nav \
                    $(COMMON_INCLUDE_PATH)/pn \
                    $(COMMON_INCLUDE_PATH)/sensetouch \
                    $(LOCAL_PATH)/../../include \
                    frameworks/native/libs/binder/include \

LOCAL_SHARED_LIBRARIES := liblog \
                          libutils \
                          libhidlbase \
                          libhidltransport \
                          com.fingerprints.extension@1.0

include $(BUILD_STATIC_LIBRARY)
