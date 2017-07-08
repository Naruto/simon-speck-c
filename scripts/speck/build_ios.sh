#!/bin/sh -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

LIBNAME="libspeck.a"

pushd ${BASEDIR} > /dev/null
    # build
    /bin/rm -rf build_iphone
    /bin/rm -rf build_iphone64
    /bin/rm -rf build_iphone_sim
    /bin/rm -rf build_iphone_sim64
    /bin/rm -rf build_iphone_fat
    /bin/mkdir build_iphone
    /bin/mkdir build_iphone64
    /bin/mkdir build_iphone_sim
    /bin/mkdir build_iphone_sim64
    /bin/mkdir build_iphone_fat

    cd build_iphone
    cmake .. -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ -DENABLE_NEON=ON
    cmake --build . 
    cd ..

    cd build_iphone64
    cmake .. -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS64 -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/ -DENABLE_NEON=ON
    cmake --build .
    cd ..

    #cd build_iphone_sim
    #cmake .. -DENABLE_STATIC=ON -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/
    #cmake --build .
    #cd ..

    cd build_iphone_sim64
    cmake .. -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/
    cmake --build .
    cd ..

    cd build_iphone_fat
    libtool -static ../build_iphone64/${LIBNAME} ../build_iphone/${LIBNAME} ../build_iphone_sim64/${LIBNAME} -o ./${LIBNAME}
    cd ..

    # deploy
    /bin/mkdir -p libs/ios
    /bin/cp build_iphone_fat/${LIBNAME} libs/ios
popd > /dev/null
