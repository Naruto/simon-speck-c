#!/bin/sh -xe

if [ $# -eq 0 ]; then
    exit 0
fi

qemu-mips-static -L /usr/mips-linux-gnu $@
