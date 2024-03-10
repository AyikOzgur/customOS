# **customOS**

**v1.0.0**

------

# Overview

CustomOS is a minimalist, customized operating system designed specifically for Raspberry Pi devices. It diverges from traditional approaches by eschewing comprehensive build systems like Yocto, Buildroot, or Busybox. Instead, it leverages only the Linux kernel and bespoke C implementations to achieve its objectives.

This experimental project prioritizes customization and efficiency over feature richness, aiming to create a lean and purpose-driven OS tailored to the specific needs of Raspberry Pi users. While not striving to become a fully operational OS, CustomOS offers a platform for building minimalist yet functional operating environments on Raspberry Pi devices.

Key Features:

- Utilizes only the Linux kernel and custom C implementations.
- Excludes complex build systems like Yocto, Buildroot, and Busybox.
- Primarily focuses on customization and efficiency.

# Versions

**Table 1** - Library versions.

| Version | Release date | What's new                                                   |
| ------- | ------------ | ------------------------------------------------------------ |
| 1.0.0   | 10.03.2024   | First version.                                               |

# Compile linux kernel

Raspberry pi [website](https://www.raspberrypi.com/documentation/computers/linux_kernel.html) explains compiling linux kernel for raspberry pi well. You can follow instructions from there but if you are planing to try different configurations


# customOS internals

## customShell

Works with console output, including kernel outputs from boot. Requires a UART connection to Raspberry Pi for usage. This shell provides kernel messages too that means if you have any problem about some hardware drivers, it can be useful to see kernel output.


## remoteShell

Facilitates remote access over UDP sockets, providing an alternative to console-based interaction. Requires an Ethernet connection between the Raspberry Pi and the host machine. Users can choose between these shell implementations based on their connectivity preferences and hardware setup, enabling flexible usage scenarios tailored to individual needs. In order to use remoteShell, you must use remoteTerminal app in host machine to access raspberry's remoteShell.
