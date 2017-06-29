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
emcmake cmake -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release ..
make -j8
popd > /dev/null

# create bundle file
/bin/rm -rf libs/emscripten
/bin/mkdir -p libs/emscripten
/bin/cp ./build/libspeck.bc libs/emscripten

popd > /dev/null
