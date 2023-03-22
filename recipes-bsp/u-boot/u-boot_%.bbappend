FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# our u-boot 2019.07 is forked, branched and patched for the support
SRCREV = "62c904685730ba8b2418129bee405e3a2dc172e6"

SRC_URI = "git://github.com/embed-me/u-boot.git;branch=v2019.07-ebaz4205;protocol=https"
SRC_URI[md5sum] = "8e306fc668e78544a040cc0f7c5bdbba"

LIC_FILES_CHKSUM = "file://Licenses/README;md5=30503fd321432fc713238f582193b78e"
SRC_URI:append = " \
  file://0001-Make-this-work-in-2023.patch \
  "

inherit xilinx-platform-init

DEPENDS += "virtual/xilinx-platform-init"

HAS_PLATFORM_INIT += " zynq_ebaz4205_config"

do_configure:prepend() {
  mkdir -p ${S}/board/embedme/ebaz4205/${MACHINE}
  for i in ${PLATFORM_INIT_FILES}; do
    cp ${PLATFORM_INIT_STAGE_DIR}/$i ${S}/board/embedme/ebaz4205/${MACHINE}/
  done
  chmod 664 ${S}/board/embedme/ebaz4205/${MACHINE}/*
}

PR = "r1"
