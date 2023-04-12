require recipes-core/images/ebaz4205-image-standard-wic.bb

IMAGE_INSTALL += "\
  mpv \
  qtbase \
                 "
TOOLCHAIN_TARGET_TASK += "serial-dev lvgl-dev dma-proxy-dev"
TOOLCHAIN_HOST_TASK += "nativesdk-cmake"
