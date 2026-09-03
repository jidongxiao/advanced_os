# Programming Assignment: Authenticated Reboot System Call Guard

## Objective

In this assignment, you will extend the Linux reboot system call mechanism to require a mandatory, dynamic "passphrase" string before allowing an OS reboot. 

You will implement a kernel module that intercepts system call execution, safely transfers memory across the user/kernel boundary, authenticates user credentials, and enforces security rules without modifying or recompiling the base kernel.

## Background

The Linux reboot system call signature is defined as:
  ```c
  long sys_reboot(int magic1, int magic2, unsigned int cmd, void __user *arg);
  ```

When performing a system restart (cmd = LINUX_REBOOT_CMD_RESTART), the 4th parameter (arg) is a pointer to user-space memory containing the authentication passphrase string. Your kernel module reads this pointer to verify the passphrase before allowing the reboot to proceed.

## Task Requirements

Task 1: Kernel Module (reboot_guard.c). You must create a kernel module that intercepts calls to sys_reboot. Your module must enforce the following validation logic:
  - Compare the copied string against a secret passphrase defined in your module:
    - If the passphrase matches: Log an authorization success message to dmesg and allow the system call to proceed.
    - If the passphrase does not match: Log an authentication failure warning to dmesg and force the system call to return -EINVAL.

Task 2: User-Space Client (reboot_auth.c). Write a C user-space client that accepts a passphrase as a command-line argument and attempts to reboot the system via raw syscall(). Requirements:  
  - Usage syntax: 
  ```bash
  # ./reboot_auth <passphrase>  
  ```

  - If no passphrase argument is provided on the command line, pass NULL as the 4th argument to syscall().  
  - Inspect the return code and print descriptive error messages using perror() or strerror() based on the value of errno.
  - The user-space client program (reboot_auth) must act strictly as a pass-through interface. It must not hardcode, define, or attempt to validate the passphrase locally, nor should it branch logic based on the passphrase string. Its sole responsibility is to collect the passphrase argument supplied by the user (e.g., via argv[1]) and pass its memory address directly to the kernel via the sys_reboot system call. All policy decisions, string comparisons, and access enforcement must occur entirely within kernel space inside your kernel module.

**Note**: In this assignment, you are recommended to use strncpy_from_user() instead of copy_from_user(). These two functions are similar. Use copy_from_user() for fixed-size C structures or primitive data types (e.g., int, struct foo). Use strncpy_from_user() for null-terminated strings, because it safely stops at \0 without over-reading user memory. Here are the prototype of these two functions:

```c
/* Standard memory copy for raw data / structs */
unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);

/* Bounded string copy for null-terminated C-strings */
long strncpy_from_user(char *dst, const char __user *src, long count);
```

## Example Program and the Magic Number Story

- You may find this [example program](./example) helpful.

- You can read the story [here](magic.md) if you want to find out the meaning of the magic numbers.

## Test Cases

Your submission will be evaluated against three distinct test scenarios on both 
unprivileged and root execution contexts:

- Test Case 1: Missing Passphrase
  ```bash
  # sudo ./reboot_auth
  ```
  Expected Result: Fails. System stays up.

- Test Case 2: Incorrect Passphrase
  ```bash
  # sudo ./reboot_auth "WRONG_PASSPHRASE"
  ```
  Expected Result: Fails. System stays up.

- Test Case 3: Correct Passphrase
  ```bash
  # sudo ./reboot_auth "Correct Passphrase"
  ```
  Expected Result: Success. The kernel module logs success and system reboot proceeds.

## Submission

Submit the following files on Submitty:
  1. reboot_guard.c  — Source code for your kernel module.
  2. reboot_auth.c   — Source code for your user-space test client.
  3. Makefile        — Build script that compiles both targets via 'make'.
  4. README.txt      — Brief description of your implementation, your chosen correct passphrase, and your test results for each of the 3 test cases.

## Due Date

09/21/2026, 11:59pm.
