#
# Copyright (c) 2018 Fingerprint Cards AB <tech@fingerprints.com>
#
# All rights are reserved.
# Proprietary and confidential.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Any use is subject to an appropriate license granted by Fingerprint Cards AB.
#
# SConscript
#
# Build script for Fingerprint Trusted Application
#

CUR_DIR := $(GET_LOCAL_DIR)

# Set the root folder. This needs to be adjusted to customer's environment
# As we symlink our source folder into devtools folder we need the full path
# here and not the cwd.

# Check debug enablers
ifneq ($(FPC_CONFIG_DEBUG), 0)
#MODULE_CFLAGS += -D_TEE_DEBUG_ -DFPC_CONFIG_DEBUG

    # Log levels for tzfingerprint:
    # 'error'                - error prints
    # 'error_info'           - error and info prints
    # 'error_info_debug'     - error, info and debug prints
FPC_TA_LOG_LEVEL = 'error_info_debug'

ifeq ($(FPC_TA_LOG_LEVEL), 'error')
MODULE_CFLAGS += -DFPC_TA_ERROR_LOG
else

ifeq ($(FPC_TA_LOG_LEVEL), 'error_info')
MODULE_CFLAGS += -DFPC_TA_ERROR_LOG -DFPC_TA_INFO_LOG
else
ifeq ($(FPC_TA_LOG_LEVEL), 'error_info_debug')
MODULE_CFLAGS += -DFPC_TA_ERROR_LOG -DFPC_TA_INFO_LOG -DFPC_TA_DEBUG_LOG
endif
endif

endif

    # Set below flag to 1 for debugging heap usage
BUILD_FPC_TZAPP_WITH_HEAP_DEBUG = 0
ifeq ($(BUILD_FPC_TZAPP_WITH_HEAP_DEBUG), 1)
MODULE_CFLAGS += -DFPC_TA_HEAP_DEBUG
endif
    # Set below flag to 1 for debugging malloc/free usage
BUILD_FPC_TZAPP_WITH_MALLOC_DEBUG = 0
ifeq ($(BUILD_FPC_TZAPP_WITH_MALLOC_DEBUG), 1)
MODULE_CFLAGS += -DFPC_TA_DEBUG_MALLOC
endif
    # Set below flag to 1 for debugging timestamp function
BUILD_FPC_TZAPP_WITH_TIMESTAMP_DEBUG = 0
ifeq ($(BUILD_FPC_TZAPP_WITH_TIMESTAMP_DEBUG), 1)
MODULE_CFLAGS += -DFPC_TA_DEBUG_TIMESTAMP
endif
    # Set below flag to 1 for debugging SFS performance
BUILD_FPC_TZAPP_WITH_SFS_PERFORMANCE_DEBUG = 1
ifeq ($(BUILD_FPC_TZAPP_WITH_SFS_PERFORMANCE_DEBUG), 1)
MODULE_CFLAGS += -DFPC_TA_DEBUG_SFS_PERFORMANCE
endif
    # Set below flag to 1 for building with SFS stubs
BUILD_FPC_TZAPP_WITH_SFS_STUBS = 0
ifeq ($(BUILD_FPC_TZAPP_WITH_SFS_STUBS), 1)
MODULE_CFLAGS += -DFPC_TA_SFS_STUBS
endif
endif

# Check if to allow error logging in release build
ifeq ($(FPC_CONFIG_DEBUG), 0)
ifneq ($(FPC_CONFIG_LOGGING_IN_RELEASE), 0)
MODULE_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE -DFPC_TA_ERROR_LOG
endif
endif

# Include paths
MODULE_INCLUDES += $(MODULE)/interface
MODULE_INCLUDES += $(MODULE)/interface/platform/tos
MODULE_INCLUDES += $(MODULE)/secure/inc
MODULE_INCLUDES += $(MODULE)/secure/inc/debug
MODULE_INCLUDES += $(MODULE)/secure/inc/engineering
MODULE_INCLUDES += $(MODULE)/secure/inc/fs
MODULE_INCLUDES += $(MODULE)/secure/inc/hw_auth
MODULE_INCLUDES += $(MODULE)/secure/lib
MODULE_INCLUDES += $(MODULE)/secure/platform/tos/inc
#MODULE_INCLUDES += $(MODULE)/secure/platform/tos/inc/qc_auth


# Add files to be compiled into TA binary and compiler flags
MODULE_SRCS += $(MODULE)/secure/src/fpc_ta_router.c
MODULE_SRCS += $(MODULE)/secure/src/fpc_ta_common.c
MODULE_SRCS += $(MODULE)/secure/src/fpc_ta_bio.c
MODULE_SRCS += $(MODULE)/secure/src/fpc_ta_sensor.c
MODULE_SRCS += $(MODULE)/secure/src/fpc_db.c
MODULE_SRCS += $(MODULE)/secure/src/fpc_crypto.c
MODULE_SRCS += $(MODULE)/secure/src/kpi/fpc_ta_kpi.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_spi.c
#MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_mem.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/app_main.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_crypto_tos.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_utils.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_log.c

ifneq ($(FPC_CONFIG_PRIV_HEAP_DEBUG), 0)
MODULE_CFLAGS += -DFPC_CONFIG_PRIV_HEAP_DEBUG
endif

ifneq ($(FPC_CONFIG_LOGGING_IN_RELEASE_FILE), 0)
MODULE_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_FILE
MODULE_CFLAGS += -DFPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE=$(FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE)
endif

ifneq ($(filter 0, $(FPC_CONFIG_PRIV_HEAP_DEBUG) $(FPC_CONFIG_LOGGING_IN_RELEASE_FILE)),)
MODULE_SRCS += $(MODULE)/secure/src/debug/fpc_debug.c
endif

ifneq ($(FPC_CONFIG_PRIV_HEAP_DEBUG_LOG), 0)
MODULE_CFLAGS += -DFPC_CONFIG_PRIV_HEAP_DEBUG_LOG
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_debug_mem_log.c
endif


ifneq ($(FPC_CONFIG_TA_FS), 0)
MODULE_CFLAGS += -DFPC_CONFIG_TA_FS
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_fs_fts.c
MODULE_SRCS += $(MODULE)/secure/src/fs/fpc_ta_fs.c

ifneq ($(FPC_CONFIG_DEBUG), 0)
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/fpc_fs_fts_unencrypted.c
endif

else

ifneq ($(FPC_CONFIG_TA_DB_BLOB), 0)
MODULE_CFLAGS += -DFPC_CONFIG_TA_DB_BLOB
MODULE_SRCS += $(MODULE)/secure/src/db_blob/fpc_ta_db_blob.c
endif

endif

ifneq ($(FPC_CONFIG_HW_AUTH), 0)
MODULE_CFLAGS += -DFPC_CONFIG_HW_AUTH
MODULE_SRCS += $(MODULE)/secure/src/hw_auth/fpc_ta_hw_auth.c
MODULE_SRCS += $(MODULE)/secure/platform/tos/src/hw_auth/fpc_ta_hw_auth_tos.c
endif

ifneq ($(FPC_CONFIG_SENSORTEST), 0)
MODULE_CFLAGS += -DFPC_CONFIG_SENSORTEST
MODULE_SRCS += $(MODULE)/secure/src/sensortest/fpc_ta_sensortest.c
endif

ifneq ($(FPC_CONFIG_ENGINEERING), 0)
MODULE_CFLAGS += -DFPC_CONFIG_ENGINEERING
MODULE_SRCS += $(MODULE)/secure/src/engineering/fpc_ta_engineering.c
endif

ifneq ($(FPC_CONFIG_NAVIGATION), 0)
MODULE_CFLAGS += -DFPC_CONFIG_NAVIGATION
MODULE_SRCS += $(MODULE)/secure/src/navigation/fpc_ta_navigation.c
endif


ifeq ($(FPC_CONFIG_LIVENESS_DETECTION), 'on')
MODULE_CFLAGS += -DFPC_CONFIG_LIVENESS_DETECTION_ENABLED
else
ifeq ($(FPC_CONFIG_LIVENESS_DETECTION), 'app_only')
MODULE_CFLAGS += -DFPC_CONFIG_LIVENESS_DETECTION_APP_ONLY
endif
endif

ifneq ($(FPC_CONFIG_IDENTIFY_AT_ENROL), 0)
MODULE_CFLAGS += -DFPC_CONFIG_IDENTIFY_AT_ENROL
endif

ifdef $(FPC_CONFIG_SPI_BUS_NUM)
MODULE_CFLAGS += -DFPC_CONFIG_SPI_BUS_NUM=$(FPC_CONFIG_SPI_BUS_NUM)
endif

ifneq ($(FPC_CONFIG_APNS), 0)
MODULE_CFLAGS += -DFPC_CONFIG_APNS
MODULE_SRCS += $(MODULE)/secure/src/fpc_ta_pn.c
endif

ifneq ($(FPC_CONFIG_ALLOW_PN_CALIBRATE), 0)
MODULE_CFLAGS += -DFPC_CONFIG_ALLOW_PN_CALIBRATE
endif

MODULE_DEPS += \
	lib/trusty_syscall_x86
