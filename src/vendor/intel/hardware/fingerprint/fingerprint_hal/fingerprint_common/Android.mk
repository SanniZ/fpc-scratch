#
# Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fpc_hal_common
LOCAL_MODULE_OWNER := fpc
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(filter 6.% 7.%, $(PLATFORM_VERSION)),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_CFLAGS := -Wall -Werror \
    -DLOG_TAG='"fpc_fingerprint_hal"' \
    -DFPC_TZ_APP_NAME="\"fpctzappfingerprint\""

LOCAL_CONLYFLAGS := -std=c99

LOCAL_SRC_FILES := fpc_tee_hal.c \
                   fpc_worker.c \
                   fpc_hal_input_device.c

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))
PLATFORM_VERSION_MINOR := $(word 2, $(subst ., ,$(PLATFORM_VERSION)))

LOCAL_CFLAGS += -DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR) \
                -DPLATFORM_VERSION_MINOR=$(PLATFORM_VERSION_MINOR)

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS    += -DFPC_CONFIG_HW_AUTH
endif

ifeq ($(FPC_CONFIG_ALLOW_PN_CALIBRATE),1)
LOCAL_CFLAGS           += -DFPC_CONFIG_ALLOW_PN_CALIBRATE
LOCAL_SRC_FILES        += pn/fpc_hal_ext_calibration.c
endif

ifeq ($(FPC_CONFIG_APNS),1)
LOCAL_CFLAGS           += -DFPC_CONFIG_APNS
LOCAL_SRC_FILES        += pn/fpc_hal_ext_recalibration.c \
                          pn/fpc_hal_pn.c
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/pn
endif

LOCAL_CFLAGS += -DPN_CALIBRATION_PATH='"$(FPC_CONFIG_PN_CALIBRATION_PATH)"'

ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_ENGINEERING
LOCAL_CFLAGS         += -DPN_CALIBRATION_DEBUG_PATH='"$(FPC_CONFIG_PN_CALIBRATION_DEBUG_PATH)"'
LOCAL_SRC_FILES      += fpc_hal_ext_engineering.c
endif

ifeq ($(FPC_CONFIG_SENSORTEST),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SENSORTEST
LOCAL_SRC_FILES += fpc_hal_ext_sensortest.c
endif

# Enable Qualcomm authentication framework support
ifeq ($(FPC_CONFIG_QC_AUTH),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_QC_AUTH
LOCAL_SRC_FILES      += fpc_hal_ext_authenticator.c
endif

# Navigation
ifeq ($(FPC_CONFIG_NAVIGATION),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_NAVIGATION
LOCAL_SRC_FILES      += nav/fpc_hal_navigation.c
endif

# SW Sense Touch
ifeq ($(FPC_CONFIG_NAVIGATION_FORCE_SW),1)
LOCAL_CFLAGS          += -DFPC_CONFIG_NAVIGATION_FORCE_SW=1
SENSE_TOUCH := 1
endif

# HW Sense Touch
ifeq ($(FPC_CONFIG_FORCE_SENSOR),1)
LOCAL_CFLAGS         += -DFPC_CONFIG_FORCE_SENSOR=1
SENSE_TOUCH := 1
endif

# Sense Touch Calibration
ifeq ($(SENSE_TOUCH),1)
LOCAL_CFLAGS         += -DSENSE_TOUCH_CALIBRATION_PATH='"$(FPC_CONFIG_SENSE_TOUCH_CALIBRATION_PATH)"'
LOCAL_SRC_FILES      += sensetouch/fpc_hal_ext_sense_touch.c \
                        sensetouch/fpc_hal_sense_touch.c
LOCAL_C_INCLUDES     += $(LOCAL_PATH)/sensetouch
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/../include \
                    $(LOCAL_PATH)/nav \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc \
                    $(LOCAL_PATH)/../../fingerprint_tac/interface/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/hw_auth/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/sensortest/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/engineering/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/kpi/ \
                    $(LOCAL_PATH)/../../fingerprint_tac/normal/inc/navigation/

LOCAL_SHARED_LIBRARIES := liblog \
                          libutils

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
ifeq ($(FPC_TEE_RUNTIME), QSEE)
LOCAL_SHARED_LIBRARIES +=  libQSEEComAPI
else ifeq ($(FPC_TEE_RUNTIME), TOS)
LOCAL_SHARED_LIBRARIES += libtrusty
endif

LOCAL_STATIC_LIBRARIES += libcutils fpc_tac

include $(BUILD_STATIC_LIBRARY)
