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
cmake -DENABLE_TESTING=ON -DCMAKE_BUILD_TYPE=Release ..
make -j8
make test
popd > /dev/null

# create bundle file
/bin/rm -rf libs/macos
/bin/mkdir -p libs/macos
/bin/mkdir -p libs/macos/speck.bundle
/bin/mkdir -p libs/macos/speck.bundle/Contents/MacOS
/bin/cp ./build/libspeck.dylib libs/macos/speck.bundle/Contents/MacOS/speck

popd > /dev/null
