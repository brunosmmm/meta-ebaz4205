DESCRIPTION = "Binwalk"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=65bbee055d3ea3bfc475f07aecf4de64"
SRC_URI = "git://github.com/ReFirmLabs/binwalk.git;branch=master;protocol=https"
SRCREV = "be738a52e09b0da2a6e21470e0dbcd5beb42ed1b"

S = "${WORKDIR}/git"

inherit setuptools3

RDEPENDS_${PN} = "python3-pycrypto \
                  squashfs-tools \
                  mtd-utils \
                  python3-capstone \
                  gzip \
                  bzip2 \
                  tar \
                  lzop \
                  srecord \
                  sleuthkit \
                  "