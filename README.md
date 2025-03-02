# Compiling kernel

```bash
git clone -b master --depth 1 https://github.com/torvalds/linux
cd linux

# Set default configs
make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 defconfig

# If you want to configure kernel uncomment following line
# make CROSS_COMPILE=aarch64-linux-gnu- ARCH=arm64 menuconfig

# compile kernel
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs

# Compile kernel module
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=output modules_install

# copy the lib folder from output/lib into rootfs
# copy kernel image from /arch/arm64/boot/Image into boot partition
# copy also proper device tree file from /linux/arch/arm64/boot/dts into boot partition.
```

Note this is compiling 64 bit kernel. If you want to compile 32 bit, you need to call with device default config file instead of defconfig. If mainline stream does not have your device defconfigs, most probably vendor provides kernel source code. Also do not forget to change ARCH and CROSS_COMPILE environment variables to:
```bash
ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
```

Also if you want to get compressed kernel image you can replace `Image` with `zImage`.



# U-boot

- Install dependencies
    ```bash
    sudo apt-get install swig python3-dev device-tree-compiler gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
    ```

- Clone source code of u-boot and TF-A(Required for 64 bit systems)
    ```bash
    cd
    mkdir uboot
    cd uboot

    git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    git clone git://git.denx.de/u-boot.git

    cd trusted-firmware-a 
    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=sun50i_h616 DEBUG=1
    cp build/sun50i_h616/debug/bl31.bin ~/uboot/u-boot/

    cd ~/uboot/u-boot/
    make CROSS_COMPILE=aarch64-linux-gnu- <your device name>_defconfig
    ```

    For PLAT argument of trusted firmware please see the [website](https://trustedfirmware-a.readthedocs.io/en/latest/plat/index.html) for your platform.


## How to install uboot to sd card

This example shows installing uboot binary that includes spl to sd card for sunxi boards.
```bash
card=/dev/sda
ubootPath=~/uboot/u-boot
# Copy uboot with spl to proper location
sudo dd if=${ubootPath}/u-boot-sunxi-with-spl.bin of=${card} bs=1024 seek=8
```

**I assume that copying uboot into sd card may be more platform dependent.**


## How to compile boot.scr for uboot

`Example of boot.cmd`
```bash

echo "Setting up variables..."
# Define kernel and DTB filenames
setenv kernel_image Image                          # Kernel file name
setenv fdt_file sun50i-h618-orangepi-zero2w.dtb    # DTB file name

# Define memory load addresses
setenv kernel_addr_r 0x42000000     # Kernel memory location
setenv fdt_addr_r 0x45000000        # DTB memory location

# Get Sd card second partition partuuid
part uuid mmc 0:2 rootPartitionUUID
echo "Root Partition UUID: ${rootPartitionUUID}"

echo "Loading Linux Kernel..."
load mmc 0:1 ${kernel_addr_r} ${kernel_image}    # Load Kernel

echo "Loading Device Tree..."
load mmc 0:1 ${fdt_addr_r} ${fdt_file}           # Load DTB

echo "Setting up boot arguments..."
setenv bootargs "console=ttyS0,115200 root=PARTUUID=${rootPartitionUUID} rw rootwait init=/bin/init"

echo "Booting Linux..."
booti ${kernel_addr_r} - ${fdt_addr_r}  # If kernel is zImage use bootz, or if it is uImage use bootm
```

Command to compile boot.cmd into boot.scr : 
```bash
mkimage -A arm64 -T script -C none -n "Boot Script" -d boot.cmd boot.scr
```
End then copy boot.scr into boot partition. U-boot will find it from there when boot is started. 

# Compiling glic 
Another crucial component of a linux system is,  Of course C libraries. 

1. Obtain source code of glibc from [website](https://ftp.gnu.org/gnu/glibc/) and extract it.
2. Following script shows a simple way of cross compiling glibc :
    ```bash
    #!/bin/bash

    set -e  # Exit immediately on error

    # Define environment variables
    export TARGET=aarch64-linux-gnu
    export PREFIX=<Enter target rootfs>
    export SYSROOT=/usr/$TARGET
    export PATH=$PREFIX/bin:$PATH

    # Ensure the compiler exists
    if ! command -v ${TARGET}-gcc &> /dev/null; then
        echo "Error: Cross compiler ($TARGET-gcc) not found!"
        exit 1
    fi

    # Create install directory
    mkdir -p $PREFIX

    # Define cross-compiler
    export CC=${TARGET}-gcc
    export CXX=${TARGET}-g++
    export AR=${TARGET}-ar
    export RANLIB=${TARGET}-ranlib

    # Create and enter build directory
    mkdir -p build
    cd build

    # Configure Glibc for cross-compilation
    ../configure --host=$TARGET \
        --build=$(../scripts/config.guess) \
        --target=$TARGET \
        --prefix=$PREFIX \
        --with-headers=$SYSROOT/include \
    
    # Compile Glibc
    make -j$(nproc)
    make install
    ```

# Initramfs 

Initramfs will not be used mainly in this documentation but in case anyone will be interested in. I am going to mention how I get it work in raspberry pi board.

1. Create folders for initramfs.
    ```bash
    mkdir -p initramfs/{bin,sbin,etc,proc,sys,usr/bin,usr/sbin}
    ```

2. Put whatever you want in initramfs, don't forget init process.

3. Create initramfs.
    ```bash
    find . | cpio -o -H newc | gzip > ../initramfs.img.gz
    ```

4. Copy **initramfs.img.gz** into boot partition.

5. Add following line to **config.txt**
    ```xml
    initramfs initramfs.img.gz followkernel
    ```
    
**In the future when I will learn how to run initramfs with uboot I will add it here. So it can be more general not just for raspberry pi.**

*NOTE: Please remember if you have initramfs, you need to change root file system to sd card in init process.*



# Creating device nodes 

Most probably creating the device nodes manually wont be needed since 64-bit kernel uses devtmpfs (at least for me) by default. However assuming that your kernel does not do it automatically you can do it in init process. However if you don't want this you may need to create the some simple device files manually in your host machine by inserting sd card. Some examples : 
```bash
targetRootFile=<Enter target root file system>

sudo rm -rf ${targetRootFile}/dev/mmcblk0*
sudo mknod ${targetRootFile}/dev/mmcblk0 b 179 0
sudo mknod ${targetRootFile}/dev/mmcblk0p1 b 179 1
sudo mknod ${targetRootFile}/dev/mmcblk0p2 b 179 2
sudo chmod 660 ${targetRootFile}/dev/mmcblk0p*

sudo mknod ${targetRootFile}/dev/stdin c 1 0
sudo chmod 666 ${targetRootFile}/dev/stdin
sudo mknod ${targetRootFile}/dev/stdout c 1 1
sudo chmod 666 ${targetRootFile}/dev/stdout
sudo mknod ${targetRootFile}/dev/stderr c 1 2
sudo chmod 666 ${targetRootFile}/dev/stderr
```
