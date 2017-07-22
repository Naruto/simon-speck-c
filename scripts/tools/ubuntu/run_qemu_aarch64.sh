#!/bin/sh -xe

if [ $# -eq 0 ]; then
    exit 0
fi

qemu-aarch64-static -L /usr/aarch64-linux-gnu  $@
