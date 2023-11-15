SUMMARY = "AXI-MUX Kernel Module"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = " \
        file://Makefile \
        file://dma-proxy.c \
        file://dma-proxy.h \
        file://COPYING \
        "

S = "${WORKDIR}"

PR = "r0"

do_install:append() {
  install -d ${D}${includedir}
  install -m 0644 ${S}/dma-proxy.h ${D}${includedir}
}
