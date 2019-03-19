#!/bin/bash

PRJ_HOME=$(pwd)
PATCH_HOME=$(cd `dirname $0`; pwd)

DIFF_DIR=${PATCH_HOME}/diff
SRC_DIR=${PATCH_HOME}/src

function applypatch(){
    dir=$(dirname $1)
    git_dir=${PRJ_HOME}/${dir:${#DIFF_DIR}}
    cd $git_dir
    date_value=`grep "Date: " $1`
    patchexist=`git log --pretty=format:"Date: %aD" | grep "$date_value"`

    file=$1
    if [ -z "$patchexist" ] ; then
        git am $1 >& /dev/null
        if [ $? == 0 ]; then
            echo "  patch appied   : ${file:${#DIFF_DIR}}"
        else
            echo "  conflict found : ${file:${#DIFF_DIR}}"
            git am --abort >& /dev/null
        fi
    fi
}

function scan_patch_files(){
    for file in `ls $1`
    do
        if [ -d $1"/"$file ] ; then
            scan_patch_files $1"/"$file
        else
            if [ ${file##*.} = "patch" ] ; then
                applypatch $1"/"$file
            fi
        fi
    done
}

function copy_src_files(){
    for file in `ls $1`
    do
        path=$1/$file
        dir=${path:${#SRC_DIR}}
        cp -rf $1/$file/. ${PRJ_HOME}/$dir
        echo "    src copied   : ${dir}"
    done
}

echo "Apply patches ..."

scan_patch_files ${DIFF_DIR}
copy_src_files ${SRC_DIR}

echo "Apply patches done."

