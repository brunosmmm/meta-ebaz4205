require recipes-core/images/ebaz4205-image-standard-wic.bb

IMAGE_INSTALL += "\
  mpv \
  qtbase \
  libsdl2 \
  libsdl2-gfx \
  libsdl2-image \
  gstreamer1.0 \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav \
                 "
TOOLCHAIN_TARGET_TASK += "serial-dev lvgl-dev dma-proxy-dev"
TOOLCHAIN_HOST_TASK += "nativesdk-cmake"
