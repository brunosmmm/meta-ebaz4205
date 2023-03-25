FILESEXTRAPATHS:prepend := "${THISDIR}/config:${THISDIR}/files:"

# LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
SRC_URI:append = " \
                 file://bsp/net/eth.scc \
                 file://bsp/fs/mtd.scc \
                 file://bsp/other/other.scc \
                 file://bsp/debug/debug.scc \
                 file://bsp/net/wifi.cfg \
                 file://i2c.cfg \
                 file://input.cfg \
                 file://serial.cfg \
                 "

KERNEL_FEATURES:append = " \
                         eth.scc \
                         mtd.scc \
                         other.scc \
                         debug.scc \
                         "

LINUX_VERSION = "6.1"
KERNEL_VERSION_SANITY_SKIP="1"
KBRANCH = "master"
SRCREV = "f9c8e14ae03c937a79e1c904d004d80a0db3647e"

PR="r3"