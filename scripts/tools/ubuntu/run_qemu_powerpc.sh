#!/bin/sh -xe

if [ $# -eq 0 ]; then
    exit 0
fi

qemu-ppc-static -L /usr/powerpc-linux-gnu $@
