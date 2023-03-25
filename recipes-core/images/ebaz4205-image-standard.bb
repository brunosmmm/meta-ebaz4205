require recipes-core/images/ebaz4205-image-minimal.bb

DESCRIPTION = "This image adds additional packages to form a standard image."

IMAGE_INSTALL += "\
        net-tools \
        nfs-utils \
        mtd-utils \
        squashfs-tools \
        devmem2 \
        strace \
	      uioctl \
        ldd \
        vim \
        i2c-tools \
        python3 \
        python3-pip \
        openocd \
        sigrok-cli \
        packagegroup-core-full-cmdline \
        spitools \
        htop \
        iotop \
        valgrind \
        git \
        wget \
        flashrom \
        fish \
        tmux \
        screen \
        packagegroup-core-buildessential \
        gdb \
        kernel-devsrc \
        distro-feed-configs \
        tzdata \
        binwalk \
        aximux \
        kernel-modules \
        libgpiod \
        libgpiod-tools \
        libevdev \
        python3-evdev \
        python3-gpiod \
        python3-ipython \
        "

IMAGE_FEATURES += "ssh-server-openssh \
         package-management \
         tools-debug \
         "

IMAGE_LINGUAS = "en-us"