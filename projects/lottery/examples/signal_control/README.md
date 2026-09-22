# Kernel Process Control via Signals Demo

This example kernel module demonstrates how a kernel module can control user-space process execution by sending kernel-privileged signals (`SIGSTOP` and `SIGCONT`) directly to a target process's `struct task_struct`.

## What This Module Demonstrates

* **PID to Task Struct Resolution:** Resolving a PID integer to a `struct task_struct` using `find_get_pid()` and `get_pid_task()`, with reference cleanup (`put_pid()`, `put_task_struct()`).
* **Kernel-Privileged Signal Delivery (`send_sig_info`):** Using `SEND_SIG_PRIV` to send `SIGSTOP` (pause process) and `SIGCONT` (resume process) directly to user-space tasks from kernel space.
* **Process Preemption and Resumption:** Toggling execution state on a target process on a recurring timer schedule to simulate quantum-based task scheduling.

## Building and Running the Demo

### 1. Build the Kernel Module and Target Application

```bash
make
```

### 2. Start the Target Application

Run target_app in the background (or in a separate terminal) and take note of its PID:

```bash
./target_app
# Example output: [TARGET] Running with PID: 12345
```

### 3. Load the Module with target_pid

In a separate terminal, insert the module, passing the PID of target_app via the required target_pid parameter:

```bash
sudo insmod signal_control_demo.ko target_pid=12345
```

### 4. Observe Process Toggling

Check dmesg output to observe the module alternating between sending SIGSTOP and SIGCONT every 5 seconds:

```bash
sudo dmesg -w
```

You will also see target_app output freeze when paused (SIGSTOP) and resume printing when unblocked (SIGCONT).

### 5. Unload the Module and Clean Up

Unloading the module automatically sends a final SIGCONT to leave the target process running cleanly:

```bash
sudo rmmod signal_control_demo
kill -9 12345
make clean
```
