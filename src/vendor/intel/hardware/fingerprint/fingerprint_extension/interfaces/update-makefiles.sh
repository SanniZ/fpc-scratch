#!/bin/bash
# Run this script in the Android tree when changes has been made
# to any of the .hal files. This will update the makefiles and also
# generate dummy c++ implementations.

source system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
    "com.fingerprints:vendor/intel/hardware/fingerprint/fingerprint_extension/interfaces/" \
    "android.hardware:hardware/interfaces" \
    "android.hidl:system/libhidl/transport"

LOC=vendor/intel/hardware/fingerprint/fingerprint_hal/fingerprint_hwbinder/extensions
PACKAGE=com.fingerprints.extension@1.0
hidl-gen -o $LOC -Lc++-impl -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:vendor/intel/hardware/fingerprint/fingerprint_extension/interfaces/ $PACKAGE
hidl-gen -o $LOC -Landroidbp-impl -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:vendor/intel/hardware/fingerprint/fingerprint_extension/interfaces/ $PACKAGE
