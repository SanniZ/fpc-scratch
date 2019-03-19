#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on 2019-03-19

@author: Byng Zeng
"""

import os
import sys

import revertpatch as rp

VERSION = '1.0.0'
AUTHOR = 'Byng.Zeng'


# ===============================================================
# ReverFPCtPatch class
# ===============================================================
class FpcRevertPatch(object):

    GITS = {  # git : subject of stop.
        'device/intel/mixins': 'Audio: FW: Update BXT-P to 9.22.03.3662',
        'device/intel/sepolicy': 'recovery: fix sepolicy for access GPU',
        'kernel/bxt': 'i915/fb: use kthread replace async',
        'kernel/config-lts/v4.9':
        'Config: enable ipu4 early device on Gorden peak',
        'packages/services/Car':
        'Decrease the duration of activity transition.',
        'trusty/app/keymaster':
        'Revert "[trusty][app] change KEYMASTER_MAX_BUFFER_LENGTH to 68KB"',
        'trusty/app/sand': '[crypto]remove log in user build',
        'trusty/device/x86/sand': '[device]remove log in user build',
        'trusty/platform/sand':
        '[platform][sand] bypass retrieval of attkb in manufacturing mode',
        'trusty/lk/trusty':
        'Revert "[trusty][lk] change IPC_CHAN_MAX_BUF_SIZE to 68KB"',
        'vendor/intel/fw/evmm':
        'Replace TARGET_PRODUCT with TRUSTY_REF_TARGET',
        'vendor/intel/hardware/fingerprint': 'Initial empty repository',
        # 'vendor/intel/hardware/storage': '',
    }

    SRCS = [
        'kernel/bxt/drivers/fpc',
        'kernel/bxt/include/linux/wakelock.h',
        'trusty/app/sand/fingerprint',
        'trusty/app/sand/fpcfingerprint',
        'vendor/intel/hardware/fingerprint/fingerprint_extension',
        'vendor/intel/hardware/fingerprint/fingerprint_hal',
        'vendor/intel/hardware/fingerprint/fingerprint_libs',
        'vendor/intel/hardware/fingerprint/fingerprint_tac',
        'vendor/intel/hardware/fingerprint/Android.bp',
        'vendor/intel/hardware/fingerprint/Android.mk']


# ===============================================================
# main entrance
# ===============================================================
if __name__ == '__main__':
    if len(sys.argv) >= 2:
        root = os.path.abspath(sys.argv[1])
        rp.revert_patches(root, FpcRevertPatch.GITS)
        rp.remove_src_files(root, FpcRevertPatch.SRCS)
    else:
        print('Error, input root path of project.')
