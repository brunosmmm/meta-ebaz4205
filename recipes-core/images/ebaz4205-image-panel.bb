require recipes-core/images/ebaz4205-image-standard-wic.bb

IMAGE_INSTALL += "\
                 dma-proxy \
                 "
TOOLCHAIN_TARGET_TASK += "serial-dev lvgl-dev"
TOOLCHAIN_HOST_TASK += "nativesdk-cmake"
