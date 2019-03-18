#
# Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# rules.mk
#
# Build script for Fingerprint Trusted Application
#


# Set environment variables from command line arguments (or default values)

# Default values if not specified on command line
# These needs to be changed if not specified on command line
FPC_CONFIG_TA_FS_DEFAULT              = 0
FPC_CONFIG_TA_DB_BLOB_DEFAULT         = 1

FPC_CONFIG_HW_AUTH_DEFAULT            = 1

FPC_CONFIG_SENSORTEST_DEFAULT         = 0
FPC_CONFIG_ENGINEERING_DEFAULT        = 0
FPC_CONFIG_NAVIGATION_DEFAULT         = 0
FPC_CONFIG_LIVENESS_DETECTION_DEFAULT = 0
FPC_CONFIG_IDENTIFY_AT_ENROL_DEFAULT  = 1
FPC_CONFIG_LOGGING_IN_RELEASE_DEFAULT = 0
LIBFPC_NAME_DEFAULT                   = libfpc1028.a
FPC_CONFIG_PRIV_HEAP_DEBUG_DEFAULT    = 0
FPC_CONFIG_PRIV_HEAP_DEBUG_LOG_DEFAULT  = 0
FPC_CONFIG_DEBUG_DEFAULT              = 1
FPC_CONFIG_SPI_BUS_NUM_DEFAULT        = 0
FPC_CONFIG_TA_NAME_DEFAULT            = fingerprint
FPC_CONFIG_APNS_DEFAULT               = 0
FPC_CONFIG_ALLOW_PN_CALIBRATE_DEFAULT = 0
FPC_CONFIG_LOGGING_IN_RELEASE_FILE_DEFAULT          = 0
FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE_DEFAULT   = 4096

# Set below command line option for building with FTS
export FPC_CONFIG_TA_FS = $(FPC_CONFIG_TA_FS_DEFAULT)

# Set below command line option for building with DB blob (OEM crypto)
export FPC_CONFIG_TA_DB_BLOB = $(FPC_CONFIG_TA_DB_BLOB_DEFAULT)

# Set below command line option to 1 to enable HW authentication support
export FPC_CONFIG_HW_AUTH = $(FPC_CONFIG_HW_AUTH_DEFAULT)

# Set below command line option to 1 to enable Sensor Test framework
export FPC_CONFIG_SENSORTEST = $(FPC_CONFIG_SENSORTEST_DEFAULT)

# Set below command line option to 1 to enable Engineering framework
export FPC_CONFIG_ENGINEERING = $(FPC_CONFIG_ENGINEERING_DEFAULT)

# Set below command line option to 1 to enable Navigation
export FPC_CONFIG_NAVIGATION = $(FPC_CONFIG_NAVIGATION_DEFAULT)

# Set below command line option to configure liveness
# liveness configurations available:
# 'on'           - enable liveness detection always
# 'app_only'     - enable liveness detection for all other cases except unlock
export FPC_CONFIG_LIVENESS_DETECTION = $(FPC_CONFIG_LIVENESS_DETECTION_DEFAULT)

# Set below command line option to 1 to enable Identify @ Enrol
export FPC_CONFIG_IDENTIFY_AT_ENROL = $(FPC_CONFIG_IDENTIFY_AT_ENROL_DEFAULT)


# Set below command line option to 1 to enable logging in release builds
export FPC_CONFIG_LOGGING_IN_RELEASE = $(FPC_CONFIG_LOGGING_IN_RELEASE_DEFAULT)

# Set below command line option to the name of the fpc library
export LIBFPC_NAME = $(subst $\',,$(LIBFPC_NAME_DEFAULT))

# Set below command line option to 1 to enable top of the heap logging
export FPC_CONFIG_PRIV_HEAP_DEBUG = $(FPC_CONFIG_PRIV_HEAP_DEBUG_DEFAULT)

# Set below command line option to 1 to enable top of the heap logging stored to file
export FPC_CONFIG_PRIV_HEAP_DEBUG_LOG = $(FPC_CONFIG_PRIV_HEAP_DEBUG_LOG_DEFAULT)

# Set below command line option to 1 to enable the storing all errors in a file in the REE
export FPC_CONFIG_LOGGING_IN_RELEASE_FILE = $(FPC_CONFIG_LOGGING_IN_RELEASE_FILE_DEFAULT)

# Set below command line option to set the size of the buffer size when storing error to log in a file in the REE
export FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE = $(FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE_DEFAULT)

# Set below command line option to 1 for building with debug functions enabled
export FPC_CONFIG_DEBUG = $(FPC_CONFIG_DEBUG_DEFAULT)

export FPC_CONFIG_SPI_BUS_NUM = $(FPC_CONFIG_SPI_BUS_NUM_DEFAULT)

export FPC_CONFIG_TA_NAME = $(FPC_CONFIG_TA_NAME_DEFAULT)


export FPC_CONFIG_APNS = $(FPC_CONFIG_APNS_DEFAULT)

export FPC_CONFIG_ALLOW_PN_CALIBRATE = $(FPC_CONFIG_ALLOW_PN_CALIBRATE_DEFAULT)


LOCAL_DIR := $(GET_LOCAL_DIR)
$(warning fpctzappfingerprint enter =============)

export MODULE := $(LOCAL_DIR)/..

MODULE_SRCS += \
	$(LOCAL_DIR)/manifest.c \

SECURE	:= secure

MODULE_DEPS += \
	app/trusty \
	lib/libc-trusty \
	lib/rng \
	lib/storage \
	lib/keymaster

MODULE_INCLUDES += \
	$(LOCAL_DIR) \

# if use clang build, no need to add this compile flag.
MODULE_COMPILEFLAGS += \
	-mstackrealign \
	-mpreferred-stack-boundary=5 \

include $(LOCAL_DIR)/../$(SECURE)/platform/tos/build/rules.mk

#include make/module-user_task.mk

# Path to the prebuilt FPC library
XBIN_PREBUILD_LIB += platform/sand/lib/fpcfingerprint/$(LIBFPC_NAME)

include make/module.mk
$(warning  fpctzappfingerprint leave =============)

