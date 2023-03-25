SUMMARY = "Python bindings for libgpiod."
AUTHOR = "Bartosz Golaszewski <brgl@bgdev.pl>"

LICENSE = "GPL-2.0-or-later & LGPL-2.1-or-later & CC-BY-SA-4.0"
LIC_FILES_CHKSUM = " \
    file://../../LICENSES/GPL-2.0-or-later.txt;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://../../LICENSES/LGPL-2.1-or-later.txt;md5=4b54a1fd55a448865a0b32d41598759d \
    file://../../LICENSES/CC-BY-SA-4.0.txt;md5=fba3b94d88bfb9b81369b869a1e9a20f \
"

require recipes-support/libgpiod/libgpiod-src.inc


SRC_URI[sha256sum] = "f74cbf82038b3cb98ebeb25bce55ee2553be28194002d2a9889b9268cce2dd07"
S = "${WORKDIR}/libgpiod-2.0/bindings/python"

inherit setuptools3

DEPENDS += "libgpiod"
RDEPENDS:${PN} += "libgpiod (>= 2.0)"

export GPIOD_WITH_TESTS = "${@bb.utils.contains("PTEST_ENABLED", "1", "1", "0", d)}"

do_install:append() {
    # Python setuptools have an issue where they install C extensions even if
    # they're not in the list of packages to be packaged. This is how the test
    # extensions end up being installed. Remove them here.
    rm -rf ${D}${PYTHON_SITEPACKAGES_DIR}/tests/
}
