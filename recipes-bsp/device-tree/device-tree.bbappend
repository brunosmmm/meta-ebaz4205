FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

COMPATIBLE_MACHINE_ebaz4205-zynq7 = ".*"
SRC_URI_append_ebaz4205-zynq7 = " \
                  file://ebaz4205.dtsi \
                  file://floppy.dtsi \
                  "
# HACK !!!!!
do_configure_append_ebaz4205-zynq7() {
  cp ${WORKDIR}/ebaz4205.dtsi ${DT_FILES_PATH}/ebaz4205.dtsi
  cp ${WORKDIR}/floppy.dtsi ${DT_FILES_PATH}/floppy.dtsi
  echo '/include/ "ebaz4205.dtsi"' >> ${DT_FILES_PATH}/system-top.dts
  echo '/include/ "floppy.dtsi"' >> ${DT_FILES_PATH}/system-top.dts
  sed -i '/clock-frequency\s*=\s*<[0-9]\+e.\+/d' ${DT_FILES_PATH}/pl.dtsi
}

PR = "r17"