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
#cd ../cd
#arm-linux-gnueabihf-gcc -static -o cd cd.c
#cp cd ../../initramfs/bin
#cp cd /mnt/ext4/bin

# go to ls
cd ../ls
arm-linux-gnueabihf-gcc -static -o ls ls.c
cp ls ../../initramfs/bin
cp ls /mnt/ext4/bin

# go to mkdir
cd ../mkdir
arm-linux-gnueabihf-gcc -static -o mkdir mkdir.c
cp mkdir ../../initramfs/bin
cp mkdir /mnt/ext4/bin

# go to helloApp
cd ../helloApp
arm-linux-gnueabihf-gcc -o hello helloApp.c
cp hello ../../initramfs/home
sudo cp hello /mnt/ext4/home
sudo cp hello /mnt/ext4/etc
sudo cp hello /mnt/ext4/home/ozgur


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