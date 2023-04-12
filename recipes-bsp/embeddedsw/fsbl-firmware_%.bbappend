FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
  file://te_fsbl_hooks.h \
  file://te_fsbl_hooks.c \
  file://te_fsbl_hooks_te0727demo1.h \
  file://te_fsbl_hooks_te0727demo1.c \
  file://0001-Insert-TE-hooks.patch \
  "

PR = "r3"

do_configure:prepend() {
 cp ${WORKDIR}/te_fsbl* ${S}/lib/sw_apps/${APP_DIR}/src
}