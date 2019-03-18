#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

EXTENSION_PATH := $(call my-dir)

PLATFORM_VERSION_MAJOR := $(word 1, $(subst ., ,$(PLATFORM_VERSION)))

ifeq ($(PLATFORM_VERSION_MAJOR),6)
ENABLE_LEGACY_EXTENSION=1
endif

ifeq ($(PLATFORM_VERSION_MAJOR),7)
ENABLE_LEGACY_EXTENSION=1
endif

ifeq ($(ENABLE_LEGACY_EXTENSION),1)
include $(EXTENSION_PATH)/api_legacy/Android.mk
else
include $(EXTENSION_PATH)/api/Android.mk
include $(EXTENSION_PATH)/interfaces/Android.mk
endif

