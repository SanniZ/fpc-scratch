#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# lib_fpc_tac: shared library
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := fpc_tac
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := fpc
LOCAL_CFLAGS := -Wall -O2 -DLOG_TAG='"fpc_tac"' -std=c11 -Werror

ifneq ($(filter 7.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
endif

ifneq ($(filter 6.%,$(PLATFORM_VERSION)),)
FPC_SYSTEM_LIB := true
endif

ifeq ($(FPC_SYSTEM_LIB),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_C_INCLUDES := \
                    $(LOCAL_PATH)/inc/ \
                    $(LOCAL_PATH)/inc/android \
                    $(LOCAL_PATH)/../interface \
                    hardware/libhardware/include

LOCAL_SRC_FILES := \
                   src/fpc_irq_device.c \
                   src/fpc_sysfs.c \
                   src/fpc_tee.c \
                   src/kpi/fpc_tee_kpi.c \
                   ../interface/fpc_error_str.c

LOCAL_CFLAGS += -DFPC_CONFIG_KEYMASTER_APP_PATH='"$(FPC_CONFIG_KEYMASTER_APP_PATH)"'
LOCAL_CFLAGS += -DFPC_CONFIG_KEYMASTER_NAME='"$(FPC_CONFIG_KEYMASTER_NAME)"'

ifndef FPC_CONFIG_NO_SENSOR
LOCAL_SRC_FILES  += src/fpc_tee_sensor.c
else
LOCAL_SRC_FILES  += src/fpc_tee_sensor_dummy.c
endif

ifndef FPC_CONFIG_NO_ALGO
LOCAL_SRC_FILES  += src/fpc_tee_bio.c
else
LOCAL_SRC_FILES  += src/fpc_tee_bio_dummy.c
endif

ifdef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SPI_RESET
endif

ifdef FPC_CONFIG_NORMAL_SENSOR_RESET
ifndef FPC_CONFIG_NORMAL_SPI_RESET
LOCAL_SRC_FILES += src/fpc_reset_device.c
endif
LOCAL_CFLAGS += -DFPC_CONFIG_NORMAL_SENSOR_RESET
endif

ifeq ($(FPC_CONFIG_DEBUG),1)
LOCAL_CFLAGS += -DFPC_DEBUG_LOGGING -DFPC_CONFIG_DEBUG
endif

ifeq ($(FPC_CONFIG_TA_FS),1)
LOCAL_SRC_FILES += src/fpc_tee_secure_storage.c
endif

ifeq ($(FPC_CONFIG_TA_DB_BLOB),1)
LOCAL_SRC_FILES += src/fpc_tee_host_storage.c
endif

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_CFLAGS += -DFPC_CONFIG_HW_AUTH
LOCAL_SRC_FILES += src/hw_auth/fpc_tee_hw_auth.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/hw_auth
endif

ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_SRC_FILES += src/engineering/fpc_tee_engineering.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/engineering
LOCAL_CFLAGS += -DFPC_CONFIG_ENGINEERING
endif

ifeq ($(FPC_CONFIG_SENSORTEST),1)
LOCAL_SRC_FILES += src/sensortest/fpc_tee_sensortest.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/sensortest
LOCAL_CFLAGS += -DFPC_CONFIG_SENSORTEST
endif

ifeq ($(FPC_CONFIG_NAVIGATION),1)
LOCAL_SRC_FILES += src/navigation/fpc_tee_nav.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/navigation
LOCAL_CFLAGS += -DFPC_CONFIG_NAVIGATION
endif

ifeq ($(FPC_CONFIG_FINGER_LOST_INTERRUPT),1)
LOCAL_CFLAGS += -DFPC_CONFIG_FINGER_LOST_INTERRUPT=1
endif

ifdef FPC_CONFIG_AFD_STAY_IN_SLEEP
LOCAL_CFLAGS += -DFPC_CONFIG_AFD_STAY_IN_SLEEP
endif

ifdef FPC_CONFIG_LOGGING_IN_RELEASE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE
ifdef FPC_CONFIG_LOGGING_IN_RELEASE_FILE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_FILE
LOCAL_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE=$(FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE)
endif
endif

ifdef FPC_CONFIG_APNS
LOCAL_CFLAGS    += -DFPC_CONFIG_APNS
LOCAL_SRC_FILES += src/fpc_tee_pn.c
endif

ifdef FPC_CONFIG_ALLOW_PN_CALIBRATE
LOCAL_CFLAGS    += -DFPC_CONFIG_ALLOW_PN_CALIBRATE
endif

ifeq ($(FPC_CONFIG_FORCE_SENSOR),1)
LOCAL_CFLAGS += -DFPC_CONFIG_FORCE_SENSOR=1
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils

LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../interface \
    $(LOCAL_PATH)/inc/hw_auth \
    $(LOCAL_PATH)/inc/sensortest \
    $(LOCAL_PATH)/inc/engineering \
    $(LOCAL_PATH)/inc/kpi \
    $(LOCAL_PATH)/inc/navigation

ifeq ($(FPC_TEE_RUNTIME), TBASE)
include $(LOCAL_PATH)/../normal/platform/tbase/tbase.mk
else ifeq ($(FPC_TEE_RUNTIME), QSEE)
include $(LOCAL_PATH)/../normal/platform/qsee/qsee.mk
else ifeq ($(FPC_TEE_RUNTIME), ANDROID)
include $(LOCAL_PATH)/../normal/platform/android/droid.mk
else ifeq ($(FPC_TEE_RUNTIME), ISEE)
LOCAL_CFLAGS += -DFPC_CONFIG_TEE_ISEE
include $(LOCAL_PATH)/../normal/platform/isee/isee.mk
else ifeq ($(FPC_TEE_RUNTIME), TOS)
LOCAL_SHARED_LIBRARIES += libtrusty
include $(LOCAL_PATH)/../normal/platform/tos/tos.mk
else
$(error "Unknown FPC_TEE_RUNTIME=$(FPC_TEE_RUNTIME)")
endif

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_EXPORT_C_INCLUDE_DIRS)

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := fpc_tee_test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := fpc

ifeq ($(filter 6.% 7.%, $(PLATFORM_VERSION)),)
# In Android O we should place the library in /vendor/lib64 insteadof /system/lib64
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_C_INCLUDES := \
                    $(LOCAL_PATH)/inc/ \
                    $(LOCAL_PATH)/inc/kpi/ \
                    $(LOCAL_PATH)/inc/android \
                    $(LOCAL_PATH)/../interface \
                    $(LOCAL_PATH)/inc/engineering \
                    $(LOCAL_PATH)/inc/sensortest

LOCAL_SRC_FILES := \
                   src/test/fpc_tee_test.c \
                   src/test/fpc_tee_db_blob_test.c

LOCAL_CFLAGS := -Wall -Wshadow -Wunused -Wunused-result -std=c99 -Wextra \
                -Wimplicit-function-declaration -Wconversion -DLOG_TAG='"fpc_tee_test"'

ifeq ($(FPC_CONFIG_ENGINEERING),1)
LOCAL_CFLAGS += -DFPC_CONFIG_ENGINEERING
endif

ifeq ($(FPC_CONFIG_SENSORTEST),1)
LOCAL_CFLAGS += -DFPC_CONFIG_SENSORTEST
endif

ifeq ($(FPC_CONFIG_FORCE_SENSOR),1)
LOCAL_CFLAGS += -DFPC_CONFIG_FORCE_SENSOR=1
endif

LOCAL_STATIC_LIBRARIES := fpc_tac

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl

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
