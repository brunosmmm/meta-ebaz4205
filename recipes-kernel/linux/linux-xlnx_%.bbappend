FILESEXTRAPATHS_prepend := "${THISDIR}/config:${THISDIR}/files:"

# LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"
SRC_URI_append = " \
                 file://bsp/net/eth.scc \
                 file://bsp/fs/mtd.scc \
                 file://bsp/other/other.scc \
                 file://bsp/debug/debug.scc \
                 file://0001-ASoC-xlnx-Get-only-capture-or-playback-path-working.patch \
                 "

KERNEL_FEATURES_append = " \  
                         eth.scc \
                         mtd.scc \
                         other.scc \
                         debug.scc \
                         "

LINUX_VERSION = "5.4"
KERNEL_VERSION_SANITY_SKIP="1"
KBRANCH = "xlnx_rebase_v5.4"
SRCREV = "1ad9b65f0dbe2b3ecc22a498e75a48d8ee39bbda"

PR="r1"