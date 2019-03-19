#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on 2019-03-15

@author: Byng Zeng
"""

import os
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
