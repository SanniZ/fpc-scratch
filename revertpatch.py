#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on 2019-03-15

@author: Byng Zeng
"""

import os
import sys
import subprocess

VERSION = '1.0.2'
AUTHOR = 'Byng.Zeng'


# ===============================================================
# API functions
# ===============================================================
def execute_shell(cmd):
    return subprocess.check_output(cmd, shell=True) if cmd else None


# revert all of patch.
def revert_patches(path, gits):
    for git, subject in gits.items():
        get_log = 'git log --pretty=format:%s'
        reset_hard = 'git reset --hard HEAD~'
        git_path = os.path.join(path, git)
        os.chdir(git_path)
        while True:
            res = execute_shell(get_log)
            res = list(res.decode('utf-8').split('\n'))[0]
            if res == subject:
                break
            else:
                print('revert patch : %s : %s' % (git, res))
                execute_shell(reset_hard)


def remove_src_files(path, srcs):
    for src in srcs:
        p = os.path.join(path, src)
        if os.path.exists(p):
            print('remove files :', src)
            execute_shell('rm -rf %s' % p)


# ===============================================================
# ReverFPCtPatch class
# ===============================================================
class ReverFPCtPatch(object):

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
    try:
        root = os.path.abspath(sys.argv[1])
    except IndexError:
        print('Error, no root path of source code!')
    else:
        revert_patches(root, ReverFPCtPatch.GITS)
        remove_src_files(root, ReverFPCtPatch.SRCS)
