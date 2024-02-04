#!/bin/sh

# mount sd card
sudo mkdir /mnt/fat32
sudo mkdir /mnt/ext4

sudo mount -t vfat /dev/sda1 /mnt/fat32
sudo mount -t ext4 /dev/sda2 /mnt/ext4

# start from init
cd ./init
arm-linux-gnueabihf-gcc -static -o init init.c
cp init ../../initramfs/

# go to cd
cd ../my_cd
arm-linux-gnueabihf-gcc -static -o my_cd my_cd.c
cp my_cd ../../initramfs/bin
cp my_cd /mnt/ext4/bin

# go to ls
cd ../my_ls
arm-linux-gnueabihf-gcc -static -o my_ls my_ls.c
cp my_ls ../../initramfs/bin
cp my_ls /mnt/ext4/bin

# go to mkdir
cd ../my_mkdir
arm-linux-gnueabihf-gcc -static -o my_mkdir my_mkdir.c
cp my_mkdir ../../initramfs/bin
cp my_mkdir /mnt/ext4/bin

# prepare sd card device node and then initramfs compressed
cd ../../initramfs
sudo rm -rf ./dev/mmcblk0*
sudo mknod ./dev/mmcblk0 b 179 0
sudo mknod ./dev/mmcblk0p1 b 179 1
sudo mknod ./dev/mmcblk0p2 b 179 2
sudo chmod 660 ./dev/mmcblk0p*

find . | cpio -o -H newc | gzip > ../initramfs.img.gz
cd ..
cp initramfs.img.gz /mnt/fat32

# umount sd card partitions.
sudo umount /mnt/fat32
sudo umount /mnt/ext4