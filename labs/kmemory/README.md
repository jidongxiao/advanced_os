# Lab 2 - Accessing Kernel Memory

## Objective

In the previous lecture, we discussed system calls and why they are necessary. User programs run with limited privileges and therefore cannot directly access protected resources, such as kernel memory and hardware. In this lab, you will see this protection boundary in action.

You will demonstrate that:

- A user-space program cannot directly access a kernel memory address.
- Attempting to do so results in a segmentation fault.
- Kernel code can access the same memory successfully.

This lab provides a concrete reason why system calls are needed: when a user program needs a service or resource that it is not permitted to access directly, it must request the kernel to perform the operation on its behalf. Here, we will demonstrate this principle using kernel memory.

## Files

The following files are provided:

- [kmemory.c](kmemory.c)
- [user_read.c](user_read.c)
- [kernel_read.c](kernel_read.c)
- [Makefile](Makefile)

Save these files in your lab directory. You do not need to modify any of these files.

## Step 1 — Compile the Programs

Open a terminal and navigate to the lab directory. Compile the provided programs:

```bash
$ make
```

You should see the kernel build system compile kmemory.c and kernel_read.c, followed by gcc compiling user_read.c. After a successful build, verify that the following files exist:

```bash
kernel_read.ko kmemory.ko user_read
```

The .ko files are Linux kernel modules. user_read is a normal user-space executable.

## Step 2 — Load the Kernel Memory Module

Load the first kernel module:

```bash
$ sudo insmod kmemory.ko
```

This module allocates a piece of kernel memory and stores a known value in it. The module also exposes the address of this memory through:

```bash
/proc/kernel_memory_address
```

## Step 3 — Find the Kernel Memory Address

Read the exposed kernel address:

```bash
$ cat /proc/kernel_memory_address
```

You should see an address similar to:

```bash
ffff8df600b82ce8
```

The exact address will be different on your system. Record this address. You will use the same address in the next two steps.

## Step 4 — Verify the Kernel Module's Output

The kernel module also prints the allocated address and stored value to the kernel log. Run:

```bash
$ sudo dmesg | tail -n 5
```

You should see output similar to:

```bash
Allocated kernel memory at ffff8df600b82ce8
Value = 0x123456789abcdef0
```

The exact address should match the address obtained in Step 3.

The value should be:

```bash
0x123456789abcdef0
```

Take note of both the address and value.

## Step 5 — Attempt to Read the Kernel Memory from User Space

Now run the ordinary user-space program:

```bash
$ ./user_read
```

You should see output similar to:

```bash
Kernel address: 0xffff8df600b82ce8
Trying to read kernel memory...
Segmentation fault (core dumped)
```

The program should terminate with a segmentation fault.

This is the expected result.

The program knows the exact kernel virtual address, but it still cannot access the memory because it is running in user space.

## Step 6 — Load the Kernel Reader Module

Now use the kernel module kernel_read.ko to access the same address.

Pass the kernel address obtained in Step 3 as the module parameter.

For example, if your address was:

```bash
ffff8df600b82ce8
```

run:

```bash
$ sudo insmod kernel_read.ko address=0xffff8df600b82ce8
```

Replace the address with the address from your own system.

If the module loads successfully, there should be no error message.

## Step 7 — Verify that the Kernel Can Read the Memory

Check the kernel log:

```bash
$ sudo dmesg | tail -n 5
```

You should see output similar to:

```bash
Reading kernel address: ffff8df600b82ce8
Value = 0x123456789abcdef0
```

Notice that:

- The address is the same address obtained in Step 3.
- The kernel successfully reads the value.
- The value is: 0x123456789abcdef0

Therefore, the same memory access that caused the user-space program to receive a segmentation fault succeeds when performed by kernel code.

## Step 8 — Clean Up

Remove the second kernel module:

```bash
$ sudo rmmod kernel_read
```

Then remove the first kernel module:

```bash
$ sudo rmmod kmemory
```

Verify that the kernel memory module has been removed:

```bash
$ ls /proc/kernel_memory_address
```

The /proc/kernel_memory_address entry should no longer exist.

You can also clean the compiled files:

```bash
$ make clean
```

## Submission

Submit screenshots demonstrating the three required observations below.

- Screenshot 1: /proc/kernel_memory_address showing the exposed kernel address.

- Screenshot 2: ./user_read showing the segmentation fault.

- Screenshot 3: sudo dmesg | tail -n 5 showing the kernel module (kernel_read.ko) successfully reading the same address and printing the expected value.

**Note**, it is okay if you use just one screenshot which shows all of the above 3 observations. Here is an example screenshot which demonstrates all 3 observations.

Due Date: 09/17/2026, 11:59pm. Each lab has a maximum of 5 points. Late submissions will be accepted within 3 days after the deadline, with a 1-point penalty applied to the earned grade.
