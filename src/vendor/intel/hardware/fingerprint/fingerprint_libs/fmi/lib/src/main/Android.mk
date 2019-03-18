#
# Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com.fingerprints.fmi

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    $(call all-java-files-under, java) \

LOCAL_DEX_PREOPT := false

include $(BUILD_JAVA_LIBRARY)