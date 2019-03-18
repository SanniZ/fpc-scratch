#
# Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com.fingerprints.extension

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    src/com/fingerprints/extension/authenticator/IFingerprintAuthenticator.aidl \
    src/com/fingerprints/extension/authenticator/IVerifyUserCallback.aidl \
    src/com/fingerprints/extension/calibration/IFingerprintCalibration.aidl \
    src/com/fingerprints/extension/calibration/ICalibrationCallback.aidl \
    src/com/fingerprints/extension/engineering/IFingerprintEngineering.aidl \
    src/com/fingerprints/extension/engineering/IImageInjectionCallback.aidl \
    src/com/fingerprints/extension/engineering/IImageSubscriptionCallback.aidl \
    src/com/fingerprints/extension/engineering/ICaptureCallback.aidl \
    src/com/fingerprints/extension/navigation/IFingerprintNavigation.aidl \
    src/com/fingerprints/extension/sensortest/IFingerprintSensorTest.aidl \
    src/com/fingerprints/extension/sensortest/ICaptureCallback.aidl \
    src/com/fingerprints/extension/sensortest/ISensorTestCallback.aidl \
    src/com/fingerprints/extension/recalibration/IRecalibrationCallback.aidl \
    src/com/fingerprints/extension/recalibration/IFingerprintRecalibration.aidl \
    src/com/fingerprints/extension/sensetouch/IFingerprintSenseTouch.aidl \
    $(call all-java-files-under, src) \

LOCAL_DEX_PREOPT := false

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := com.fingerprints.extension.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions

LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
