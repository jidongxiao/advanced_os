# System Call Monitoring via Kretprobes and User-Space Interfacing

## OVERVIEW

This sample package demonstrates how to instrument kernel system call execution at 
runtime without recompiling the Linux kernel. It consists of two components:
  1. A Kernel Module (reboot_monitor.c) that attaches a return probe 
     (kretprobe) to the sys_reboot kernel symbol.
  2. A User-Space Client (reboot_test.c) that issues a raw system call to 
     trigger the kernel handler directly.

This design illustrates how user-space software crosses the user/kernel boundary 
and how kernel modules can audit system call results using high-level kernel APIs.

## KERNEL MODULE EXPLANATION (reboot_monitor.c)

The module monitors the execution outcome of sys_reboot using the Linux kretprobe (Kernel Return Probes) tracing framework. A kretprobe is a dynamic tracing feature provided by the Linux kernel that lets developers inspect and instrument a function as it finishes executing and returns control back to its caller.

Key Mechanisms:
  - Architecture Target Abstraction:
    The target symbol differs between processor architectures:
      * x86_64: __x64_sys_reboot
      * arm64:  __arm64_sys_reboot
    The module uses preprocessor directives (#if defined) to select the correct 
    symbol name automatically at compile time.

  - Kretprobe Return Handler (reboot_ret_handler):
    Runs immediately after sys_reboot finishes executing, right before control 
    returns to user space.
      - regs_return_value(regs): A portable kernel macro that extracts the 
        syscall's return value from the CPU's return register without needing 
        architecture-specific code.
      - current: A pointer to the task_struct of the process that called the 
        system call.
      - current_uid(): Retrieves the user credential structure. __kuid_val() 
        converts this into a numerical User ID (UID).

  - Security Auditing:
    If the system call returns -EPERM (Operation Not Permitted), the module 
    emits a warning to the kernel log via pr_warn(), recording the process name 
    (current->comm), Process ID (current->pid), and UID.


## USER-SPACE CLIENT EXPLANATION (reboot_test.c)

This program invokes the reboot system call directly.

Key Mechanisms:
  - Raw System Call Wrapper:
    syscall(SYS_reboot, magic1, magic2, cmd, arg) directly loads arguments into 
    the architecture's system call registers and triggers a software interrupt/
    trap into kernel mode.

  - Magic Constant Verification:
    Passing LINUX_REBOOT_MAGIC1 (0xfee1dead) and LINUX_REBOOT_MAGIC2 (672274793) 
    allows execution to pass the kernel's initial argument validation checks.

  - Error Handling:
    If an unprivileged user (UID != 0) invokes the system call, the kernel's 
    ns_capable(CAP_SYS_BOOT) check fails, returning -1 to user space and setting 
    errno to EPERM (Operation not permitted).


## COMPILATION AND TESTING WORKFLOW

- Build both the user application and kernel module:
   ```bash
   $ make
   ```

- Load the kernel module:
   ```bash
   $ sudo insmod reboot_monitor.ko
   ```

- Execute the user-space test client as an unprivileged user:
   ```bash
   $ ./reboot_test
   Output: Reboot failed: Operation not permitted
   ```

- Inspect the kernel logs to verify interception:
   ```bash
   $ sudo dmesg | tail -n 5
   Output: [RebootMonitor] Process 'reboot_test' (PID 14591, UID 1000) called 
           reboot(). Kernel returned: -1
           [RebootMonitor] Security alert: Unauthorized reboot attempt detected!
   ```

5. Unload the module when finished:
   ```bash
   $ sudo rmmod reboot_monitor
   ```
