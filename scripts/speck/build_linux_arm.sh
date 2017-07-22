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
cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_VERSION=1  -DCMAKE_SYSTEM_PROCESSOR=arm -DCMAKE_CROSSCOMPILING_EMULATOR=${BASEDIR}/scripts/tools/ubuntu/run_qemu_arm.sh -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=arm-linux-gnueabi-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabi-g++ -DENABLE_NEON=ON -DENABLE_TESTING=ON ..
make VERBOSE=1
ctest
popd > /dev/null

/bin/mkdir -p libs/linux
/bin/cp ./build/libspeck*.so libs/linux 
popd > /dev/null
