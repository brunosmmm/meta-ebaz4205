FILESEXTRAPATHS:append := "${THISDIR}/files:"

SRC_URI:append = " file://fstab"

do_install:append () {
        install -d ${D}/media/mmcblk0p1
}