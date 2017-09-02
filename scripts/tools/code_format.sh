#!/bin/sh -ex

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/../..
BASEDIR=`cd ${BASEDIR} && pwd -P`

format_all_source_files() {
    pushd ${BASEDIR} > /dev/null

    find src -iname '*.[c|h|cpp|cxx|cc]' -exec clang-format -i -style=file {} \;
    find test -iname '*.[c|h|cpp|cxx|cc]' -exec clang-format -i -style=file {} \;

    popd > /dev/null # ${BASEDIR}
}

main() {
    if [ $# -eq 0 ]; then
        format_all_source_files
    else
        clang-format -i -style=file $@
    fi
}

main $@
