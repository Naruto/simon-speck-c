#!/bin/sh -xe

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

pushd ${BASEDIR} > /dev/null
/bin/rm -rf build
/bin/mkdir build
# build
pushd build > /dev/null
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
popd > /dev/null

# create bundle file
/bin/rm -rf Assets/Unagi/Plugins/Mac/bisque_ws.bundle
/bin/mkdir -p Assets/Unagi/Plugins/Mac/bisque_ws.bundle
/bin/mkdir -p Assets/Unagi/Plugins/Mac/bisque_ws.bundle/Contents/MacOS
/bin/cp ${MODULEDIR}/bisque/build/bisque/libbisque_ws.dylib Assets/Unagi/Plugins/Mac/bisque_ws.bundle/Contents/MacOS/bisque_ws

/bin/rm -rf
popd > /dev/null
