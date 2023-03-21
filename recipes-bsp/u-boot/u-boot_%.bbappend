FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# our u-boot 2019.07 is forked, branched and patched for the support
SRCREV = "62c904685730ba8b2418129bee405e3a2dc172e6"

SRC_URI:remove = "git://git.denx.de/u-boot.git "
SRC_URI:prepend = "git://github.com/embed-me/u-boot.git;branch=v2019.07-ebaz4205;protocol=https"
SRC_URI[md5sum] = "8e306fc668e78544a040cc0f7c5bdbba"

SRC_URI:append = " \
               file://ps7_init_gpl.c \
               file://ps7_init_gpl.h \
               "

HAS_PLATFORM_INIT += " zynq_ebaz4205_config"

do_configure:prepend() {
        mkdir -p ${S}/board/embedme/ebaz4205/${MACHINE}
        cp ${WORKDIR}/ps7_init_gpl.h ${S}/board/embedme/ebaz4205/${MACHINE}/
        cp ${WORKDIR}/ps7_init_gpl.c ${S}/board/embedme/ebaz4205/${MACHINE}/
        chmod 664 ${S}/board/embedme/ebaz4205/${MACHINE}/*
}
