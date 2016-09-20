#!/bin/sh -xe

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
    cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_NDK=${NDK_ROOT} -DANDROID_NATIVE_API_LEVEL=android-16 -DCMAKE_TOOLCHAIN_FILE=../cmake/android.toolchain.cmake -DANDROID_ABI=${arch} ..
    make
    popd > /dev/null

    # deploy
    /bin/mkdir -p libs/android/${arch}
    /bin/cp build/libspeck.so libs/android/${arch}
done
popd > /dev/null
