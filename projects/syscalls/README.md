# Programming Assignment: Authenticated Reboot System Call Guard

## OBJECTIVE

In this assignment, you will extend the Linux reboot system call mechanism to require a mandatory, dynamic "passphrase" string before allowing an OS reboot. 

You will implement a kernel module that intercepts system call execution, safely transfers memory across the user/kernel boundary, authenticates user credentials, and enforces security rules without modifying or recompiling the base kernel.

## BACKGROUND & CORE CONCEPTS

The Linux reboot system call signature is defined as:
  ```c
  long sys_reboot(int magic1, int magic2, unsigned int cmd, void __user *arg);
  ```

When performing a system restart (cmd = LINUX_REBOOT_CMD_RESTART), the 4th 
parameter (arg) is a pointer to user-space memory. 

Key Concepts Tested:
  - User/Kernel Boundary Safety: User-space memory pointers (void __user *) 
    cannot be dereferenced directly in kernel code. You must use safe kernel 
    copy routines such as copy_from_user().
  - System Call Interception: Using kernel dynamic tracing mechanisms to 
    inspect and validate arguments before a system call executes.
  - Return Value Conventions: Returning standard Linux error codes (-EPERM, 
    -EINVAL, -EFAULT) to user space upon authentication or validation failure.

## TASK REQUIREMENTS

Task 1: Kernel Module (reboot_guard.c). You must create a kernel module that intercepts calls to sys_reboot. Your module must enforce the following validation logic:
  - Check if the passphrase pointer (arg) passed in the 4th argument is NULL. If NULL, block execution and return -EINVAL.  
  - Safely copy the string from user space into a kernel-space buffer using copy_from_user().  
  - If copy_from_user() fails (returns a negative value), block execution and return -EFAULT.  
  - Compare the copied string against a secret passphrase defined in your module:
    - If the passphrase matches: Log an authorization success message to dmesg and allow the system call to proceed.
    - If the passphrase does not match: Log an authentication failure warning to dmesg and force the system call to return -EPERM.

Task 2: User-Space Client (reboot_auth.c). Write a C user-space client that accepts a passphrase as a command-line argument and attempts to reboot the system via raw syscall(). Requirements:
    - Usage syntax: ./reboot_auth <passphrase>  
    - If no passphrase argument is provided on the command line, pass NULL as the 4th argument to syscall().  
    - Inspect the return code and print descriptive error messages using perror() or strerror() based on the value of errno.

## GRADING & TEST CASES

Your submission will be evaluated against three distinct test scenarios on both 
unprivileged and root execution contexts:

- Test Case 1: Missing Passphrase
  ```bash
  # sudo ./reboot_auth
  ```
  Expected Result: Fails with errno = EINVAL (Invalid argument). System stays up.

- Test Case 2: Incorrect Passphrase
  ```bash
  # sudo ./reboot_auth "WRONG_PASSPHRASE"
  ```
  Expected Result: Fails with errno = EPERM (Operation not permitted). System stays up.

- Test Case 3: Correct Passphrase (Root Execution)
  ```bash
  # sudo ./reboot_auth "Lake America"
  ```
  Expected Result: Success. The kernel module logs success and system reboot proceeds.

## DELIVERABLES

Submit the following files on Submitty:
  1. reboot_guard.c  — Source code for your kernel module.
  2. reboot_auth.c   — Source code for your user-space test client.
  3. Makefile        — Build script that compiles both targets via 'make'.
  4. README.txt      — Brief description of your implementation, your chosen correct passphrase, and your test results for each of the 3 test cases.

## DUE DATE

TBD.
