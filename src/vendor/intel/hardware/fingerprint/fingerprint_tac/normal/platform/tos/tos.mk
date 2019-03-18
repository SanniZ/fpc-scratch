#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# Trusty specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM    := ../normal/platform/tos
LOCAL_PATH_PLATFORM_IF := ../interface/platform/tos

# =============================================================================
LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM_IF)

LOCAL_SRC_FILES += \
		$(LOCAL_PATH_PLATFORM)/src/fpc_tos_tac.c

ifeq ($(FPC_CONFIG_HW_AUTH),1)
LOCAL_SRC_FILES += $(LOCAL_PATH_PLATFORM)/src/hw_auth/fpc_tee_hw_auth_tos.c
endif

ifndef FPC_CONFIG_LKM_CLASS
FPC_CONFIG_LKM_CLASS=platform
endif

FPC_REE_DEVICE_PATH := /sys/bus/$(FPC_CONFIG_LKM_CLASS)/devices

# Path to the Kernel REE device driver sysfs interface
LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fpc_irq"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"$(FPC_REE_DEVICE_PATH)"'

LOCAL_CFLAGS += -DFPC_TAC_APPLICATION_NAME='"fpctzappfingerprint"'
LOCAL_CFLAGS += -DFPC_TAC_APPLICATION_PATH='"/vendor/firmware"'
LOCAL_CFLAGS += -Werror -Wall


LOCAL_SHARED_LIBRARIES += \
		liblog \
		libcutils \
		libtrusty

LOCAL_CFLAGS += -fstack-protector-all -fstack-protector-strong


