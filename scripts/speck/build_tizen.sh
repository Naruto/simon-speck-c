#!/bin/bash -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

pushd ${BASEDIR} > /dev/null

tizen_build() {
    SYSROOT=$1
    CC_PATH=$2
    CXX_PATH=$3
    VERSION=$4
    MODEL=$5
    TARGET=$6
    OTHER_FLAG=$7

    # cleanup
    /bin/rm -rf build
    /bin/mkdir build

    # build
    pushd build > /dev/null
    cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_VERSION=1  -DCMAKE_SYSTEM_PROCESSOR=arm -DCMAKE_SYSROOT=${SYSROOT} -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${CC_PATH} -DCMAKE_CXX_COMPILER=${CXX_PATH} $OTHER_FLAG ..
    cmake --build . --clean-first -- VERBOSE=1
    popd > /dev/null

    # deploy
    /bin/mkdir -p libs/tizen/${VERSION}/${MODEL}-${TARGET}
    /bin/cp build/libspeck.so libs/tizen/${VERSION}/${MODEL}-${TARGET}
}

/bin/rm -rf libs/tizen
/bin/mkdir -p libs/tizen

tizen_build "${HOME}/tizen-studio/platforms/tizen-3.0/mobile/rootstraps/mobile-3.0-device.core" "${HOME}/tizen-studio/tools/arm-linux-gnueabi-gcc-4.9/bin/arm-linux-gnueabi-gcc" "${HOME}/tizen-studio/tools/arm-linux-gnueabi-gcc-4.9/bin/arm-linux-gnueabi-g++" 3.0 mobile device "-DENABLE_NEON=ON"

tizen_build "${HOME}/tizen-studio/platforms/tizen-3.0/mobile/rootstraps/mobile-3.0-emulator.core" "${HOME}/tizen-studio/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-gcc" "${HOME}/tizen-studio/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-g++" 3.0 mobile emulator

popd > /dev/null
