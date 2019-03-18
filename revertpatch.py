#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on 2019-03-15

@author: Byng Zeng
"""

import os
import sys
import subprocess

VERSION = '1.0.0'
AUTHOR = 'Byng.Zeng'

prj_home = None


def execute_shell(cmd):
    return subprocess.check_output(cmd, shell=True) if cmd else None


# revert all of patch.
def revert_patches(path):
    gits = {  # git : subject of stop.
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
    for git, subject in gits.items():
        get_log = 'git log --pretty=format:%s'
        reset_hard = 'git reset --hard HEAD~'
        git_path = os.path.join(path, git)
        os.chdir(git_path)
        # print('\n----------------%s----------------' % git)
        while True:
            res = execute_shell(get_log)
            res = list(res.decode('utf-8').split('\n'))[0]
            if res == subject:
                break
            else:
                print('revert patch : %s : %s' % (git, res))
                execute_shell(reset_hard)


def remove_dir(path, d):
    p = os.path.join(prj_home, path, d)
    if os.path.exists(p):
        print('rm src files :', os.path.join(path, d))
        execute_shell('rm -rf %s' % p)


def remove_src_files(path):
    dirs = ['kernel/bxt/drivers/fpc',
            'trusty/app/sand/fingerprint',
            'trusty/app/sand/fpcfingerprint',
            'vendor/intel/hardware/fingerprint']
    for d in dirs:
        p = os.path.join(path, d)
        if os.path.exists(p):
            if 'vendor/intel/hardware/fingerprint' in p:
                remove_dir(d, 'fingerprint_extension')
                remove_dir(d, 'fingerprint_hal')
                remove_dir(d, 'fingerprint_libs')
                remove_dir(d, 'fingerprint_tac')
                remove_dir(d, 'Android.bp')
                remove_dir(d, 'Android.mk')
            else:
                remove_dir(d, '')


if __name__ == '__main__':
    try:
        prj_home = os.path.abspath(sys.argv[1])
    except IndexError:
        print('Error, no root path of source code!')
    else:
        revert_patches(prj_home)
        remove_src_files(prj_home)
