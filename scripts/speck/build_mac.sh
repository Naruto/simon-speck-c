#!/bin/bash -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

LIBNAME="libspeck.dylib"

buildmac() {
    BUILD_DIR=$1
    ADD_FLAG=$2
    # build
    /bin/rm -rf ${BUILD_DIR}
    /bin/mkdir -p ${BUILD_DIR}

    pushd ${BUILD_DIR} > /dev/null

    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=ON  ${ADD_FLAG} ${BASEDIR}
    cmake --build .
    ctest .

    popd > /dev/null
}

pushd ${BASEDIR} > /dev/null

buildmac build/build_x86_64 "-DCMAKE_OSX_ARCHITECTURES=x86_64 -DENABLE_AVX2=ON"

/bin/rm -rf build/build_macos_fat
/bin/mkdir build/build_macos_fat
pushd build/build_macos_fat > /dev/null
lipo -create ../build_x86_64/${LIBNAME} -o ./${LIBNAME}
popd > /dev/null

# create bundle file
/bin/rm -rf libs/macos
/bin/mkdir -p libs/macos
/bin/cp build/build_macos_fat/${LIBNAME} ./libs/macos
/bin/mkdir -p libs/macos/speck.bundle
/bin/mkdir -p libs/macos/speck.bundle/Contents/MacOS
/bin/cp build/build_macos_fat/${LIBNAME} libs/macos/speck.bundle/Contents/MacOS/speck

popd > /dev/null
