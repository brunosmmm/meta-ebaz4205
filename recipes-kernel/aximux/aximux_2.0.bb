SUMMARY = "AXI-MUX Kernel Module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
        file://Makefile \
        file://aximux.c \
        file://COPYING \
        "

S = "${WORKDIR}"

PR = "r1"
