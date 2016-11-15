#!/bin/bash -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

ARCHTECTURES=(
    "armeabi-v7a"
    "x86"
#    "x86_64"
#    "arm64-v8a"
)

pushd ${BASEDIR} > /dev/null
for arch in ${ARCHTECTURES[@]}; do
    # build
    /bin/rm -rf build
    /bin/mkdir build
    pushd build > /dev/null
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=16 -DCMAKE_ANDROID_ARCH_ABI=${arch} -DCMAKE_ANDROID_NDK=${NDK_ROOT} -DCMAKE_ANDROID_STL_TYPE=gnustl_static ..
    make
    popd > /dev/null

    # deploy
    /bin/mkdir -p libs/android/${arch}
    /bin/cp build/libspeck.so libs/android/${arch}
done
popd > /dev/null
