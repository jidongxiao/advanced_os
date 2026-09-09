# Privileged CPU Register Access

## Objective

This demo illustrates the **hardware privilege boundary** between user space and the Linux kernel.

A normal application runs with limited CPU privileges:

* x86-64: **CPL 3 / Ring 3**
* AArch64: **EL0**

The Linux kernel runs with higher privileges:

* x86-64: **CPL 0 / Ring 0**
* AArch64: **EL1**

We will attempt to access a privileged CPU register from both user space and kernel space and observe the difference.

## Demo

The demo automatically selects the appropriate implementation for the machine's architecture.

### x86-64

The user program attempts to read the **CR3 control register**:

```asm
mov %cr3, <general-purpose-register>
```

CR3 is a privileged x86-64 control register. A normal user program running at Ring 3 is not permitted to read it, so the CPU raises an exception and Linux terminates the process.

The kernel module performs the same operation while running at Ring 0, where the operation is permitted.

### AArch64

The user program attempts to read the **TTBR0_EL1 system register**:

```asm
mrs <general-purpose-register>, ttbr0_el1
```

`MRS` reads a system register into a general-purpose register. Access to `TTBR0_EL1` requires the appropriate privilege level, so a normal application running at EL0 cannot perform this access.

The kernel module performs the same operation while running at EL1, where it is permitted.

## Files

All four source files are provided in the same directory:

```text
Makefile
privileged_user_x86_64.c
privileged_x86_64.c
privileged_user_aarch64.c
privileged_aarch64.c
```

The Makefile automatically detects the architecture and compiles the appropriate user program and kernel module.

## Running the Demo

First compile the demo:

```bash
make
```

Run the user-space program:

```bash
./privileged_user
```

On both architectures, the user-space program should fail because it attempts to access a privileged CPU register.

Then load the kernel module:

```bash
sudo insmod privileged.ko
```

Check the kernel log:

```bash
sudo dmesg | tail
```

You should see the value read from the privileged register.

Finally, unload the module:

```bash
sudo rmmod privileged
```

## Key Observation

The **same type of privileged CPU-register access** behaves differently depending on where the code is running:

```text
             User program             Kernel module
                  │                       │
              Ring 3 / EL0            Ring 0 / EL1
                  │                       │
        privileged register      privileged register
             access                    access
                  │                       │
                  X                       ✓
             exception               succeeds
```

The CPU, not just the operating system, enforces this privilege boundary.
