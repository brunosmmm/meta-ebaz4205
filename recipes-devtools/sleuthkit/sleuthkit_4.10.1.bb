DESCRIPTION = "The sleuth kit"
LICENSE = "OTHER"

FILESEXTRAPATHS_prepend = "${THISDIR}/files:"
SRC_URI = "git://github.com/sleuthkit/sleuthkit.git;branch=develop;protocol=https"
SRC_URI_append =  " file://0001-Remove-cross-compilation-unsafe-flags.patch"

SRCREV = "e8bc8f1f9fd94f70238635e7e2ce88883bdda377"
LIC_FILES_CHKSUM = "file://licenses/Apache-LICENSE-2.0.txt;md5=3b83ef96387f14655fc854ddc3c6bd57 \
                    file://licenses/GNU-COPYING;md5=1c4c603b49b20ca5f2a761096e8daf2e \
                    file://licenses/IBM-LICENSE;md5=59f236b2177829954041b10e30e306d2 \
                    file://licenses/bsd.txt;md5=fbf4382e8f9f238aa8b486ec0654939b \
                    file://licenses/cpl1.0.txt;md5=059e8cd6165cb4c31e351f2b69388fd9 \
                    file://licenses/mit.txt;md5=54e556309c0cd88303ff7666f67bc878 \
                    "

S = "${WORKDIR}/git"

inherit pkgconfig autotools-brokensep

EXTRA_OECONF = "--disable-java --without-afflib --without-libewf"
DEPENDS = "zlib"
RDEPENDS_${PN} = "perl"

FILES_${PN} += "/usr/share/tsk/*"

do_configure() {
    # remove reference to host machine perl
    sed -i 's|AC_PATH_PROG(PERL, perl)|AC_SUBST(PERL, ${bindir}/perl)|g' ${S}/configure.ac
    ./bootstrap
    oe_runconf
}

do_compile() {
  oe_runmake all
}

do_install() {
  oe_runmake install DESTDIR=${D} SBINDIR=${sbindir} MANDIR=${mandir} INCLUDEDIR=${includedir}
}