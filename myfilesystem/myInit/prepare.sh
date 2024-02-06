#!/bin/sh

# mount sd card
sudo mkdir /mnt/fat32
sudo mkdir /mnt/ext4

sudo mount -t vfat /dev/sdb1 /mnt/fat32
sudo mount -t ext4 /dev/sdb2 /mnt/ext4

# start from init
cd ./init
arm-linux-gnueabihf-gcc -static -o init init.c
cp init ../../initramfs/

# go to ls
cd ../ls
arm-linux-gnueabihf-gcc -o ls ls.c
cp ls ../../initramfs/bin
cp ls /mnt/ext4/bin

# go to mkdir
cd ../mkdir
arm-linux-gnueabihf-gcc -o mkdir mkdir.c
cp mkdir ../../initramfs/bin
cp mkdir /mnt/ext4/bin

# go to rm
cd ../rm
arm-linux-gnueabihf-gcc -o rm rm.c
cp rm ../../initramfs/bin
cp rm /mnt/ext4/bin

# go to touch
cd ../touch
arm-linux-gnueabihf-gcc -o touch touch.c
cp touch ../../initramfs/bin
cp touch /mnt/ext4/bin

# go to helloApp
cd ../helloApp
arm-linux-gnueabihf-gcc -o hello helloApp.c
cp hello ../../initramfs/home
sudo cp hello /mnt/ext4/home
sudo cp hello /mnt/ext4/etc
sudo cp hello /mnt/ext4/home/ozgur


# prepare sd card device node and then initramfs compressed.
cd ../../initramfs
sudo rm -rf ./dev/mmcblk0*
sudo mknod ./dev/mmcblk0 b 179 0
sudo mknod ./dev/mmcblk0p1 b 179 1
sudo mknod ./dev/mmcblk0p2 b 179 2
sudo chmod 660 ./dev/mmcblk0p*

sudo rm -rf /mnt/ext4/dev/mmcblk0*
sudo mknod /mnt/ext4/dev/mmcblk0 b 179 0
sudo mknod /mnt/ext4/dev/mmcblk0p1 b 179 1
sudo mknod /mnt/ext4/dev/mmcblk0p2 b 179 2
sudo chmod 660 /mnt/ext4/dev/mmcblk0p*

sudo mknod /mnt/ext4/dev/stdin c 1 0
sudo chmod 666 /mnt/ext4/dev/stdin
sudo mknod /mnt/ext4/dev/stdout c 1 1
sudo chmod 666 /mnt/ext4/dev/stdout
sudo mknod /mnt/ext4/dev/stderr c 1 2
sudo chmod 666 /mnt/ext4/dev/stderr


find . | cpio -o -H newc | gzip > ../initramfs.img.gz
cd ..
cp initramfs.img.gz /mnt/fat32

# umount sd card partitions.
sudo umount /mnt/fat32
sudo umount /mnt/ext4