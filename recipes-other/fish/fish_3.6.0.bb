DESCRIPTION = "Fish shell"

inherit deploy cmake pkgconfig

PN = "fish"
PV = "3.6.0"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/COPYING;md5=fbac53dd4da483010e69e10c2221f02d"

SRC_URI = "git://github.com/fish-shell/fish-shell.git;protocol=https;branch=master"
SRCREV = "af833a700d35e2d2f735e7fffca6bc9415aa01fb"

S = "${WORKDIR}/git"

DEPENDS = "ncurses libpcre2 gettext"

PR = "r1"
