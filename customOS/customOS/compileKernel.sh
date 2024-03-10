#!/bin/sh

linuxPath=/home/ozgur/linux

cd $linuxPath

make -j16 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean all

# Prepare default configuration
KERNEL=kernel7
make -j16 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bcm2709_defconfig

# Enable Realtek RTL8152/RTL8153 Based USB Ethernet Adapters
scripts/config --enable CONFIG_USB_RTL8152

# Enable USB RTL8150 based ethernet device support
scripts/config --enable CONFIG_USB_RTL8150

# Enable Video4Linux core
scripts/config --enable CONFIG_VIDEO_DEV

# Enable V4L2 core
scripts/config --enable CONFIG_VIDEO_V4L2

# Enable Video4Linux drivers (replace XXX with specific drivers as needed)
scripts/config --enable CONFIG_VIDEO_BCM2835 

# Enable support for cameras using the V4L2 sub-device interface
scripts/config --enable CONFIG_VIDEO_V4L2_SUBDEV_API

# If you need specific support for media controller (optional)
scripts/config --enable CONFIG_MEDIA_CONTROLLER

# Enable BCM2835 Unicam support
scripts/config --enable CONFIG_VIDEO_BCM2835_UNICAM

# Enable BCM2835 ISP support
scripts/config --enable CONFIG_VIDEO_BCM2835_ISP

scripts/config --enable CONFIG_I2C
scripts/config --enable CONFIG_I2C_BCM2835

# Update the .config file with the new configuration, resolving any new dependencies
yes "y" | make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j16 oldconfig

# This will allow us to enable proper drivers etc with GUI.
make -j16 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig

make -j16 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules dtbs


sudo rm -rf mnt

mkdir mnt
mkdir mnt/fat32
mkdir mnt/ext4
sudo mount /dev/sda1 mnt/fat32
sudo mount /dev/sda2 mnt/ext4

sudo env PATH=$PATH make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=mnt/ext4 modules_install

sudo cp arch/arm/boot/zImage mnt/fat32/$KERNEL.img
sudo cp arch/arm/boot/dts/*.dtb mnt/fat32/
sudo cp arch/arm/boot/dts/overlays/*.dtb* mnt/fat32/overlays/
sudo cp arch/arm/boot/dts/overlays/README mnt/fat32/overlays/
sudo umount mnt/fat32
sudo umount mnt/ext4
