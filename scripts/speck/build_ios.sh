#!/bin/sh -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

LIBNAME="libspeck.a"

buildios() {
    BUILD_DIR=$1
    IOS_PLATFORM=$2
    OSX_SYSROOT=$3
    ADD_FLAG=$4
    # build
    /bin/rm -rf ${BUILD_DIR}
    /bin/mkdir -p ${BUILD_DIR}

    pushd ${BUILD_DIR} > /dev/null

    cmake -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${BASEDIR}/cmake/ios.toolchain.cmake -DIOS_PLATFORM=${IOS_PLATFORM} -DCMAKE_OSX_SYSROOT=${OSX_SYSROOT} ${ADD_FLAG} ${BASEDIR}
    cmake --build . 

    popd > /dev/null 
}

pushd ${BASEDIR} > /dev/null

    /bin/rm -rf build
    /bin/mkdir -p build

    # iPhone armv7
    buildios build/build_iphone OS /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ "-DENABLE_NEON=ON"

    # iPhone armv7s
    buildios build/build_iphones OSs /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ "-DENABLE_NEON=ON"

    # iPhone arm64
    buildios build/build_iphone64 OS64 /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ "-DENABLE_NEON=ON"

    ## iPhone simulator
    buildios build/build_sim SIMULATOR /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/ "-DENABLE_BITCODE=OFF"

    # iPhone simulator64
    buildios build/build_sim64 SIMULATOR64 /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/ "-DENABLE_BITCODE=OFF"

    /bin/rm -rf build/build_iphone_fat
    /bin/mkdir build/build_iphone_fat
    pushd build/build_iphone_fat > /dev/null
    libtool -static ../build_iphone64/${LIBNAME} ../build_iphone/${LIBNAME} ../build_iphones/${LIBNAME} ../build_sim/${LIBNAME} ../build_sim64/${LIBNAME} -o ./${LIBNAME}
    popd > /dev/null

    # deploy
    /bin/mkdir -p libs/ios
    /bin/cp build/build_iphone_fat/${LIBNAME} libs/ios
popd > /dev/null
