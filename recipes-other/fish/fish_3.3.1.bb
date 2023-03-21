DESCRIPTION = "Fish shell"

inherit deploy cmake pkgconfig

PN = "fish"
PV = "3.3.1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/COPYING;md5=71c215ba7ebbd6d5bb72d650be5a2152"

SRC_URI = "git://github.com/fish-shell/fish-shell.git;protocol=https;branch=master"
SRCREV = "b2f791b577e9041fbe28042771b80146849d37af"

S = "${WORKDIR}/git"

DEPENDS = "ncurses"
