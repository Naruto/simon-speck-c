#!/bin/bash -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

pushd ${BASEDIR} > /dev/null

SYSTEM_VERSION=16

android_ndk_build() {
    SYSTEM_VERSION=$1
    ARCH=$2
    OTHER_FLAG=$3

    # build
    /bin/rm -rf build
    /bin/mkdir build
    pushd build > /dev/null
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang -DCMAKE_SYSTEM_VERSION=${SYSTEM_VERSION} -DCMAKE_ANDROID_ARCH_ABI=${ARCH} -DCMAKE_ANDROID_NDK=${NDK_ROOT} $OTHER_FLAG ..
    cmake --build .
    popd > /dev/null

    # deploy
    /bin/mkdir -p libs/android/${ARCH}
    /bin/cp build/libspeck.so libs/android/${ARCH}
}

android_ndk_build ${SYSTEM_VERSION} "armeabi-v7a" "-DCMAKE_ANDROID_ARM_NEON=ON -DENABLE_NEON=ON"
android_ndk_build ${SYSTEM_VERSION} "x86"
#    "x86_64"
#    "arm64-v8a"

popd > /dev/null
