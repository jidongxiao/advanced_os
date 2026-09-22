# Kernel Process Control via Signals Demo

This example kernel module demonstrates how a kernel module can control user-space process execution by sending kernel-privileged signals (`SIGSTOP` and `SIGCONT`) directly to a target process's `struct task_struct`.

## What This Module Demonstrates

* **PID to Task Struct Resolution:** Resolving a PID integer to a `struct task_struct` using `find_get_pid()` and `get_pid_task()`, with reference cleanup (`put_pid()`, `put_task_struct()`).
* **Kernel-Privileged Signal Delivery (`send_sig_info`):** Using `SEND_SIG_PRIV` to send `SIGSTOP` (pause process) and `SIGCONT` (resume process) directly to user-space tasks from kernel space.
* **Process Preemption and Resumption:** Toggling execution state on a target process on a recurring timer schedule to simulate quantum-based task scheduling.
