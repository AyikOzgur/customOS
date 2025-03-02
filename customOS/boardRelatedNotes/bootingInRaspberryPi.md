# Raspberry Pi Boot Process

The Raspberry Pi's default bootloader can directly boot the kernel without requiring an additional bootloader like **U-Boot**.

## Bootloader Files

To boot successfully, the following essential bootloader binaries must be copied to the **boot partition (FAT32)**:

- `bootcode.bin`
- `fixup.dat`
- `start.elf`

## Configuration Files

### `config.txt` (Bootloader Configuration)

The **config.txt** file controls boot-time settings for the Raspberry Pi firmware. Below is an example:

```ini
# Enable UART for serial debugging
enable_uart=1

# Specify the kernel image to load. If you have 64 bit kernel it should be kernel8.img
kernel=kernel7.img

# Force HDMI output even if no display is detected
hdmi_force_hotplug=1

# Set HDMI mode (group and mode settings define resolution and refresh rate)
hdmi_group=1
hdmi_mode=1
```

### `cmdline.txt` (Kernel arguments)
```init
console=serial0,115200 root=/dev/mmcblk0p2 rootfstype=ext4 rw rootwait
```
