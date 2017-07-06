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

STL_TYPES=(
    "stlport"
    "c++"
    "gnustl"
)

NDK=
if [ x$NDK_ROOT != x"" ]; then
    NDK=${NDK_ROOT}
elif [ x$ANDROID_NDK_HOME != x"" ]; then
    NDK=${ANDROID_NDK_HOME}
else
    echo "please set Android NDK path to 'NDK_ROOT' or 'ANDROID_NDK_HOME' environment variables"
    exit 1
fi

pushd ${BASEDIR} > /dev/null
for arch in ${ARCHTECTURES[@]}; do
    # build
    for stl_type in ${STL_TYPES[@]}; do
        /bin/rm -rf build
        /bin/mkdir build
        pushd build > /dev/null
        cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=16 -DCMAKE_ANDROID_ARCH_ABI=${arch} -DCMAKE_ANDROID_NDK=${NDK} -DCMAKE_ANDROID_STL_TYPE=${stl_type}_static ..
        make
        # TODO: strip
        popd > /dev/null
        # deploy
        /bin/mkdir -p libs/android/${arch}/${stl_type}
        /bin/cp build/libspeck.so libs/android/${arch}/${stl_type}
    done

done
popd > /dev/null
