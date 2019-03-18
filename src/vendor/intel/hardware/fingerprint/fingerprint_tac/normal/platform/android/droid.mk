#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# =============================================================================
# Android/REE specific includes for the TAC
# Note that LOCAL_PATH is set to the location of the normal/Android.mk that
# includes this file
# =============================================================================
LOCAL_PATH_PLATFORM := ../normal/platform/android

# =============================================================================
LOCAL_C_INCLUDES += \
		$(LOCAL_PATH)/$(LOCAL_PATH_PLATFORM)/inc

LOCAL_SRC_FILES += \
		$(LOCAL_PATH_PLATFORM)/src/fpc_droid_tac.c

# Path to the Kernel REE device driver sysfs interface
LOCAL_CFLAGS += -DFPC_REE_DEVICE_ALIAS_FILE='"modalias"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_NAME='"fpc_interrupt"'
LOCAL_CFLAGS += -DFPC_REE_DEVICE_PATH='"/sys/bus/platform/devices"'

LOCAL_SHARED_LIBRARIES := \
		lib_fpc_ta_shared

LOCAL_CFLAGS += -DFPC_TEE_ANDROID

#LOCAL_EXPORT_C_INCLUDE_DIRS += \
