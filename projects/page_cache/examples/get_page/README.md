## Virtual-to-Physical Address Translator (get_page)

### Getting the Physical Page

This example demonstrates how a kernel module can translate a user-space virtual address into the struct page representing the physical page that backs that address.

The example uses:

find_get_pid() and get_pid_task() to find a process.

get_task_mm() to obtain the process's memory descriptor.

get_user_pages_remote() to locate the physical page backing a user-space virtual address.

page_to_pfn() to obtain the page frame number (PFN).

### Compilation

Build the loadable kernel module (get_page.ko) using make:

```bash
make clean
make
```

### Usage Guide

The module accepts two parameters during insertion:

target_pid: The PID of the target process (optional; defaults to the PID of insmod if omitted).

user_address: The virtual memory address in hexadecimal format (required).

Step 1: Find a Valid Target PID and Memory Address
Inspect /proc/<PID>/maps for a target process (e.g., PID 1) to locate an active mapped virtual address range:

```bash
sudo head -n 5 /proc/1/maps
```

Example Output:

```plaintext
5c8c21847000-5c8c2184d000 r--p 00000000 08:01 145480  /usr/lib/systemd/systemd
```

Here, 0x5c8c21847000 is a valid mapped virtual address belonging to PID 1.

Step 2: Load the Module
Insert the kernel module using insmod, providing both target_pid and user_address:

```bash
sudo insmod get_page.ko target_pid=1 user_address=0x5c8c21847000
```

Note: Always specify both target_pid and user_address. Omitting target_pid causes the module to evaluate the address against insmod's own address space, resulting in insmod: ERROR: Bad address.

Step 3: View Translation Output
Inspect kernel log messages via dmesg:

```bash
sudo dmesg | tail -n 10
```

Example Output:

Plaintext
========================================
get_page Translation Result
========================================
PID:               1
Virtual address:   0x5c8c21847000
PFN:               2227804
Physical address:  0x21fe5c000
========================================
Step 4: Unload the Module
Unload the module using rmmod:

```bash
sudo rmmod get_page
```
