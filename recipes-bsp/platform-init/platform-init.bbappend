COMPATIBLE_MACHINE = "^ebaz4205-zynq7$"
DEPENDS += "unzip-native"

# NOTE: xilinx has to update upstream, files names have changed in yocto
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

# do not try to fetch platform init files
SRC_URI = ""

do_install:prepend() {
  fn=$(unzip -l ${HDF_PATH} | awk '{print $NF}' | grep "^ps7_init_gpl")
  unzip -o ${HDF_PATH} ${fn} -d ${S}
}