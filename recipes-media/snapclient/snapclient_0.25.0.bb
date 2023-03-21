DESCRIPTION = "Snapcast Client"
LICENSE = "GPLv3"
PR = "r0"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=7702f203b58979ebbc31bfaeb44f219c"

DEPENDS = "flac libvorbis libopus boost expat avahi soxr alsa-lib"
RDEPENDS_${PN} = "flac libvorbis libopus soxr expat avahi-daemon libavahi-client"

SRC_URI = "git://github.com/badaix/snapcast.git;protocol=https;tag=v0.25.0"

S = "${WORKDIR}/git"

inherit pkgconfig cmake systemd

FILES_${PN} += "${bindir} ${systemd_unitdir}/system"

do_install_prepend() {

}

do_install_append() {
  install -d ${D}${systemd_unitdir}/system
  install -m 0644 ${S}/debian/snapclient.service ${D}${systemd_unitdir}/system
}

EXTRA_OECMAKE += "-DBUILD_SERVER=no -DBUILD_CLIENT=yes"
