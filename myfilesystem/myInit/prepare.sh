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

# go to myShell
cd ../myShell
arm-linux-gnueabihf-gcc -static -o myShell myShell.c
cp myShell /mnt/ext4/bin
rm ./myShell

# go to remoteShell
cd ../remoteShell
arm-linux-gnueabihf-gcc -static -o remoteShell remoteShell.c
cp remoteShell /mnt/ext4/bin
rm ./remoteShell

# go to ls
cd ../ls
arm-linux-gnueabihf-gcc -o ls ls.c
cp ls /mnt/ext4/bin
rm ./ls

# go to mkdir
cd ../mkdir
arm-linux-gnueabihf-gcc -o mkdir mkdir.c
cp mkdir /mnt/ext4/bin
rm ./mkdir

# go to rm
cd ../rm
arm-linux-gnueabihf-gcc -o rm rm.c
cp rm /mnt/ext4/bin
rm ./rm

# go to touch
cd ../touch
arm-linux-gnueabihf-gcc -o touch touch.c
cp touch /mnt/ext4/bin
rm ./touch

# go to clear
cd ../clear
arm-linux-gnueabihf-gcc -o clear clear.c
cp clear /mnt/ext4/bin
rm ./clear

# go to cat
cd ../cat
arm-linux-gnueabihf-gcc -o cat cat.c
cp cat /mnt/ext4/bin
rm ./cat

# go to helloApp
cd ../helloApp
arm-linux-gnueabihf-gcc -o hello helloApp.c
sudo cp hello /mnt/ext4/home/ozgur
rm ./hello

# go to netto test
cd ../netto
arm-linux-gnueabihf-gcc -o netto netto.c
sudo cp netto /mnt/ext4/home/ozgur
rm ./netto

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

# create initramfs compressed file.
find . | cpio -o -H newc | gzip > ../initramfs.img.gz
cd ..
cp initramfs.img.gz /mnt/fat32
rm -rf ./initramfs.img.gz

# umount sd card partitions.
sudo umount /mnt/fat32
sudo umount /mnt/ext4