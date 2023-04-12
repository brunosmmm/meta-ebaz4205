def uenv_populate(d):
    # populate the environment values
    env = {}

    env["machine_name"] = d.getVar("MACHINE")

    env["kernel_image"] = d.getVar("KERNEL_IMAGETYPE")
    env["loadkernel"] = "load mmc 0 ${kernel_load_address} ${kernel_image}"

    env["devicetree_image"] = boot_files_dtb_filepath(d)
    env["loaddtb"] = "load mmc 0 ${devicetree_load_address} ${devicetree_image}"
    env["bootkernel"] = "run loadkernel && run loaddtb && " + uboot_boot_cmd(d) + " ${kernel_load_address} - ${devicetree_load_address}"

    env["bootargs"] = d.getVar("KERNEL_BOOTARGS")
    env["uenvcmd"] = "run bootkernel"

    bitstream, bitstreamtype = boot_files_bitstream(d)
    if bitstream:
        env["bitstream_image"] = bitstream.strip()

        # if bitstream is "bit" format use loadb, otherwise use load
        env["bitstream_type"] = "loadb" if bitstreamtype else "load"

        # load bitstream first with loadfpa
        env["fpga_config"] = "fpga ${bitstream_type} 0 ${bitstream_load_address} ${filesize}"

    env["bootdelay"] = "3"

    return env

# bootargs, default to booting with the rootfs device being partition 2 of the first mmc device
KERNEL_BOOTARGS:zynq = "earlyprintk console=ttyPS0,115200 root=/dev/mmcblk0p2 rw rootwait cma=25M"
KERNEL_BOOTARGS:zynqmp = "earlycon clk_ignore_unused root=/dev/mmcblk0p2 rw rootwait uio_pdrv_genirq.of_id=generic-uio"
