DESCRIPTION = "Fish shell"

inherit deploy cmake pkgconfig

PN = "fish"
PV = "3.6.0"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/COPYING;md5=71c215ba7ebbd6d5bb72d650be5a2152"

SRC_URI = "git://github.com/fish-shell/fish-shell.git;protocol=https;branch=master"
SRCREV = "af833a700d35e2d2f735e7fffca6bc9415aa01fb"

S = "${WORKDIR}/git"

DEPENDS = "ncurses"
