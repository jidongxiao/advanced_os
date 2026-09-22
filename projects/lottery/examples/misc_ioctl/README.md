# Misc Device & IOCTL Kernel Module Demo

This example kernel module demonstrates how to create a character device node using the kernel's `miscdevice` subsystem and exchange control data with user-space applications via `ioctl`.

## What This Module Demonstrates

* **Automatic Device Node Creation (`misc_register` / `misc_deregister`):** How to register a `/dev/` entry dynamically using `MISC_DYNAMIC_MINOR` without manually allocating major/minor device numbers.
* **Kernel File Operations (`struct file_operations`):** How to bind the `unlocked_ioctl` callback to process incoming user-space `ioctl()` system calls.
* **Safe User-Kernel Memory Transfer:** Using `copy_from_user()` and `copy_to_user()` to safely read and write memory across the user-space and kernel-space boundary.
* **Structured IOCTL Command Macro Definitions:** Using `_IOW()` and `_IOR()` magic numbers to define type-safe commands.
