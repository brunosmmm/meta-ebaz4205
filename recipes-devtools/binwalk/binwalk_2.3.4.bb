DESCRIPTION = "Binwalk"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=65bbee055d3ea3bfc475f07aecf4de64"
SRC_URI = "git://github.com/ReFirmLabs/binwalk.git;branch=master;protocol=https"
SRCREV = "cddfede795971045d99422bd7a9676c8803ec5ee"

S = "${WORKDIR}/git"

inherit setuptools3

RDEPENDS:${PN} = "python3-pycryptodome \
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