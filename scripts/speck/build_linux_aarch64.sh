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
cmake -DCMAKE_CROSSCOMPILING=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_VERSION=1  -DCMAKE_SYSTEM_PROCESSOR=arm -DCMAKE_CROSSCOMPILING_EMULATOR=${BASEDIR}/scripts/tools/ubuntu/run_qemu_aarch64.sh -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DENABLE_NEON=ON -DENABLE_TESTING=ON ..
cmake --build . --clean-first
ctest .
popd > /dev/null

/bin/mkdir -p libs/linux
/bin/cp ./build/libspeck*.so libs/linux 
popd > /dev/null
