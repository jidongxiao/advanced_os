# Ring 0 and Ring 3 Demo

This demo shows the difference between **user mode (Ring 3)** and **kernel mode (Ring 0)** on an x86-64 Linux system.

The CPU keeps track of the privilege level of the code currently executing. This is called the **Current Privilege Level (CPL)**.

The demo reads the **CS (Code Segment) register** and uses its lower two bits to determine the CPL:

```text
CPL = CS & 0x3
```

The expected values are:

```text
CPL = 3  →  Ring 3 (user mode)
CPL = 0  →  Ring 0 (kernel mode)
```

## Files

* `ring_user.c` — user-space program that reads and displays its CPL.
* `ring_kernel.c` — kernel module that reads and displays the kernel's CPL.
* `Makefile` — builds both programs.

## Build

Run:

```bash
make
```

This produces:

```text
ring_user
ring_kernel.ko
```

## 1. Check User Mode

Run the user-space program:

```bash
./ring_user
```

You should see output similar to:

```text
User program:
  CS  = 0x0033
  CPL = 3
  Running at Ring 3 (user mode)
```

The important observation is:

```text
CPL = 3
```

This shows that a normal user-space program executes at **Ring 3**.

## 2. Check Kernel Mode

Load the kernel module:

```bash
sudo insmod ring_kernel.ko
```

Then check the kernel log:

```bash
sudo dmesg | tail
```

You should see output similar to:

```text
Kernel module:
  CS  = 0x0010
  CPL = 0
  Running at Ring 0 (kernel mode)
```

The important observation is:

```text
CPL = 0
```

This shows that the Linux kernel executes at **Ring 0**.

## 3. Remove the Kernel Module

When finished:

```bash
sudo rmmod ring_kernel
```

## What This Demonstrates

The two programs execute in different privilege levels:

```text
User program
     │
     │ Ring 3
     ▼
  CPL = 3

---------------------------
   CPU privilege boundary
---------------------------

     ▲
     │ Ring 0
     │
  CPL = 0
     │
     Kernel
```

This privilege separation is fundamental to operating systems. User programs run with restricted privileges, while the kernel runs with the higher privileges needed to manage protected resources such as memory, hardware, and other system resources.

The **CS register** provides the CPU with information about the currently executing code and its privilege level. By examining its lower two bits, we can observe the CPL directly in this experiment.
