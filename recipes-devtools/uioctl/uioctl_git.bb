SUMMARY = "U-Boot uEnv.txt SD boot environment generation for Zynq targets"
SECTION = "dev"
LICENSE = "GPL-3.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-3.0-only;md5=c79ff39f19dfec6d293b95dea7b07891"

SRC_URI = "git://github.com/bnewbold/uioctl.git;protocol=https;branch=master"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

FILES:${PN} += "${bindir}"
FILES:${PN}-dbg += "${bindir}/.debug"

do_compile() {
	     ${CC} ${LDFLAGS} uioctl.c -Os -o uioctl
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 uioctl ${D}${bindir}
}