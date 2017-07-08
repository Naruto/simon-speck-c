#!/bin/bash -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

pushd ${BASEDIR} > /dev/null
/bin/rm -rf build
/bin/mkdir build
# build
pushd build > /dev/null
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++ -DENABLE_NEON=ON ..
make VERBOSE=1
ctest
popd > /dev/null

/bin/mkdir -p libs/linux
/bin/cp ./build/libspeck*.so libs/linux 
popd > /dev/null
