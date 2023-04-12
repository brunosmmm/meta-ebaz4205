FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

COMPATIBLE_MACHINE:ebaz4205-zynq7 = ".*"
SRC_URI:append:ebaz4205-zynq7 = " \
                  file://ebaz4205.dtsi \
                  file://panel.dtsi \
                  "
# HACK !!!!!
do_configure:append:ebaz4205-zynq7() {
  cp ${WORKDIR}/ebaz4205.dtsi ${DT_FILES_PATH}/ebaz4205.dtsi
  cp ${WORKDIR}/panel.dtsi ${DT_FILES_PATH}/panel.dtsi
  echo '/include/ "ebaz4205.dtsi"' >> ${DT_FILES_PATH}/system-top.dts
  echo '/include/ "panel.dtsi"' >> ${DT_FILES_PATH}/system-top.dts
  sed -i '/clock-frequency\s*=\s*<[0-9]\+e.\+/d' ${DT_FILES_PATH}/pl.dtsi
  sed -i '/fclk-enable/d' ${DT_FILES_PATH}/pcw.dtsi
}


PR = "r24"