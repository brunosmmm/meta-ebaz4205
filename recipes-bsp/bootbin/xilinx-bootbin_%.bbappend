# enable bitstream loading
BIF_BITSTREAM_ATTR = "bitstream"

# disable device-tree loading
BIF_DEVICETREE_ATTR = ""

# use u-boot, not u-boot-xlnx
BIF_SSBL_ATTR  = "u-boot"
BIF_PARTITION_IMAGE[fsbl] = "${DEPLOY_DIR_IMAGE}/fsbl-${MACHINE}.elf"

BIF_PARTITION_IMAGE[u-boot] = "${DEPLOY_DIR_IMAGE}/u-boot-${MACHINE}.elf"
BIF_PARTITION_DEPENDS[u-boot] = "virtual/bootloader:do_deploy"