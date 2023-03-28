FILESEXTRAPATHS:prepend = "${THISDIR}/files:"
SUMMARY = "Xilinx AXIDMA Driver"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4e905fb1f0726930632bec5bf36aa051"
SRC_URI = "git://github.com/bperez77/xilinx_axidma;branch=master;protocol=https"
SRCREV = "42ed91e83bc4da1e29149b2be0c6a6b8f4549222"
S = "${WORKDIR}/git"

inherit module

SRC_URI += " \
        file://Makefile \
        file://0001-Enable-Linux-5.4-Petalinux-2020.1-support.patch \
        file://0001-Import-namespace.patch \
        "

PR = "r0"

do_configure:append() {
  cp ${WORKDIR}/Makefile ${S}/
  cp ${S}/include/axidma_ioctl.h ${S}/driver
}