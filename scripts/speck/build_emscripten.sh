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
cmake --build .

# rename .c.o to .bc
ar x libspeck.a
for f in *.c.o; do
    mv $f ${f%.c.o}.bc
done
zip libspeck.zip *.bc

popd > /dev/null

/bin/rm -rf libs/emscripten
/bin/mkdir -p libs/emscripten
/bin/cp ./build/libspeck.zip libs/emscripten

popd > /dev/null
