#!/bin/sh -xe

if [ $# -eq 0 ]; then
    exit 0
fi

qemu-arm-static -L /usr/arm-linux-gnueabi $@
