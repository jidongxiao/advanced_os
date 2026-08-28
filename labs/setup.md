# Lab 1 - Course VM Setup

This document describes how to set up the **course VM**, install Ubuntu 24.04 LTS (64-bit), compile the Linux 7.0 kernel from source, and boot the VM using the newly compiled kernel.

## 1. Prerequisites

You will need:

- A computer capable of running a virtual machine.
- At least **8 GB of RAM** on the host computer.
- At least **50 GB of available disk space** on the host computer.
- Hardware virtualization support enabled on the host computer.
- A virtualization program. **VirtualBox is recommended.**
- An Internet connection for downloading Ubuntu and the Linux kernel source.

> **Important:** The course VM must run **64-bit Ubuntu 24.04 LTS**. Do not install a 32-bit version of Ubuntu.

## 2. Download Ubuntu 24.04 LTS

Download the **Ubuntu 24.04 LTS 64-bit Desktop** ISO from the official Ubuntu website.

The downloaded file should have a name similar to:

```text
ubuntu-24.04.x-desktop-amd64.iso
```

The exact version number may be different depending on the latest Ubuntu 24.04 release.

## 3. Create the Virtual Machine

Create a new virtual machine in VirtualBox.

Recommended settings:

| Setting | Value |
|---|---|
| Operating System | Ubuntu |
| Architecture | 64-bit |
| Memory | 4 GB |
| Virtual CPUs | 2 |
| Virtual Disk | 50 GB |
| Disk Type | Dynamically allocated |
| Network | NAT |
| Audio Controller | ICH AC'97 |
| Network Adapter | Intel E1000 |

A dynamically allocated 50 GB disk does **not** immediately consume 50 GB of physical disk space. It grows as data is added to the VM.

### Hardware virtualization

Hardware virtualization must be enabled on the host computer.

Depending on the host CPU, this may be called:

- Intel VT-x
- AMD-V
- Hardware Virtualization

If VirtualBox reports that hardware virtualization is unavailable, check the computer's firmware/UEFI settings.

## 4. Install Ubuntu

Start the VM and attach the Ubuntu 24.04 LTS ISO as the installation medium.

Follow the Ubuntu installation wizard.

For disk partitioning, the simplest option is:

> **Erase disk and install Ubuntu**

This option erases the **virtual disk inside the VM**, not the physical disk of your host computer.

You do not need to manually create partitions for this course.

After the installation finishes:

1. Reboot the VM.
2. Remove/eject the Ubuntu ISO when VirtualBox asks you to do so.
3. Log in to Ubuntu.

> **Important:** Make sure that you installed the **64-bit version of Ubuntu 24.04 LTS**. Do not install a 32-bit version.

## 5. Complete the Ubuntu Installation

After Ubuntu finishes installing, reboot the VM and log in to your newly installed system.

Open a terminal and verify the Ubuntu version:

```bash
lsb_release -a
```

You should see information indicating that the system is Ubuntu 24.04 LTS.

Next, verify that you are running a 64-bit system:

```bash
uname -m
```

The expected output is:

```text
x86_64
```

> **Important:** The course VM must use a 64-bit Ubuntu installation. If `uname -m` reports `i386`, `i686`, or another 32-bit architecture, you have not installed the required version of Ubuntu.

### Update Ubuntu

Before continuing with the kernel development environment, update the installed packages:

```bash
sudo apt update
sudo apt upgrade
```

If Ubuntu asks you to reboot after the upgrade, reboot the VM:

```bash
sudo reboot
```

After rebooting, verify the Ubuntu version and architecture again:

```bash
lsb_release -a
uname -m
```

At this point, the basic Ubuntu installation is complete and the VM is ready for the course development environment.

## 6. Install Kernel Development Tools

Install the packages needed to configure and compile the Linux kernel:

```bash
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev bc dwarves
```

These packages provide the compiler, build tools, kernel configuration interface, and other tools required to build the kernel.

## 7. Download Linux Kernel 7.0

Download the Linux 7.0 source code.

You should obtain a file similar to:

```bash
linux-7.0.tar.xz
```

Then extract it:

```bash
tar -xf linux-7.0.tar.xz
```

This creates a directory:

linux-7.0/

Enter the source directory:

```bash
cd linux-7.0
```

## 8. Configure the Kernel

For x86 users, before compiling the kernel, you must configure it using the **course-provided kernel configuration file**.

The instructor will provide a configuration file named:

```text
config-7.0.0-test
```

which is [here](config-7.0.0-test), copy this file into the root directory of the Linux 7.0 source tree and rename it to .config:

```bash
cp config-7.0.0-test .config
```

For arm users, you are recommended to copy the existing config file from the /boot folder of your VM.

## 9. Compile the Kernel

You can compile using:

```bash
make -j2
```

## 10. Install the Kernel Modules

After the kernel compilation completes successfully:

```bash
sudo make modules_install
```

## 11. Install the Kernel

```bash
sudo make install
```

## 12. Update GRUB

Update the GRUB bootloader configuration:

```bash
sudo update-grub
```

## 13. Reboot and Select Linux 7.0

Reboot the VM:

```bash
sudo reboot
```

## 14. Verify the Running Kernel

After Ubuntu starts, open a terminal and run:

```bash
uname -r
```


