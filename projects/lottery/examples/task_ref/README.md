# Linux Kernel Module: `task_ref_demo`

A minimal, self-contained Linux kernel module demonstrating how to safely traverse the global process list, acquire references to `struct task_struct`, and avoid **Use-After-Free (UAF)** race conditions during process termination.

---

## 1. Overview & Pedagogical Goals

In the Linux kernel, process metadata (`struct task_struct`) is dynamically allocated on the slab heap (`task_struct_cachep`). When a process terminates (`do_exit()`), its memory structures are eventually unlinked and freed back to the slab allocator.

If a custom kernel module holds a raw pointer to a `task_struct` and attempts to inspect it after the process exits, it will trigger a **kernel panic (Kernel NULL Pointer Dereference or Slab Use-After-Free)**.

This module demonstrates the canonical two-phase synchronization pattern used by core kernel subsystems:
1. **RCU Read-Side Protection:** Safe traversal of the global task list without locking up process creation/exit.
2. **Atomic Reference Counting (`get_task_struct` / `put_task_struct`):** Pinning the memory lifetime of a target `task_struct` so it remains valid outside lock boundaries, even if the underlying process exits concurrently.

---

## 2. Key Technical Concepts

### A. RCU List Traversal (`rcu_read_lock` / `rcu_read_unlock`)
The global process list in Linux is protected by Read-Copy-Update (RCU). 
* `rcu_read_lock()` disables preemption, ensuring that pointer addresses along the linked list remain valid while being traversed via `for_each_process(p)`.
* **Constraint:** Code executing inside an RCU critical section **must not sleep or block** (e.g., no `msleep()`, no `GFP_KERNEL` allocations, no `mutex_lock()`).

### B. Pinning Task Memory (`get_task_struct`)
Calling `get_task_struct(p)` increments the atomic reference counter (`p->usage`). 
* Even if the process terminates and becomes a zombie during execution, the kernel slab allocator **will not free** the `task_struct` memory block while `p->usage > 0`.
* This allows the module to safely release the RCU lock, perform sleeping work or I/O, and dereference the pointer without panic risks.

### C. Releasing Memory (`put_task_struct`)
Calling `put_task_struct(p)` decrements `p->usage`.
* Every call to `get_task_struct()` **must** be paired 1-to-1 with a corresponding `put_task_struct()`.
* Once the usage counter reaches zero, the slab memory is safely reclaimed by the kernel.

---

## 3. Building the Module

```bash
make
```

---

## 4. Usage & Verification

### Scenario 1: Inspecting a Long-Running Process (e.g., PID 1)

1. Load the kernel module specifying `target_pid=1` (typically `systemd` or `init`).

```bash
sudo insmod task_ref_demo.ko target_pid=1
```

2. Inspect the kernel log (`dmesg`).

```bash
sudo dmesg | tail -n 10
```

3. Unload the module to release the reference.

```bash
sudo rmmod task_ref_demo
sudo dmesg | tail -n 5
```

---

### Scenario 2: Verification Against Concurrency Race Condition (Target Process Dies)

This test proves that holding a reference prevents kernel panics when a process dies while being referenced by the module.

1. Launch a short-lived dummy process in the background and capture its PID.

```bash
sleep 30 &
TARGET_PID=$!
echo "Target PID is $TARGET_PID"
```

2. Load the module pointing to the target PID.

```bash
sudo insmod task_ref_demo.ko target_pid=$TARGET_PID
```

3. Force-kill the target process while the module is active.

```bash
kill -9 $TARGET_PID
```

4. Verify via `dmesg` that the module safely executed its sleep cycle and read the process state without inducing a Use-After-Free crash.

```bash
sudo dmesg | tail -n 10
```

5. Unload the module cleanly.

```bash
sudo rmmod task_ref_demo
```

---

## 6. Code Architecture Summary

```text
[ Process Context ]
       │
       ├──> rcu_read_lock()
       │      ├──> for_each_process(p)
       │      └──> get_task_struct(p)  ──> Increments p->usage (Pins memory)
       ├──> rcu_read_unlock()
       │
       ├──> [ Safe Zone: Module can msleep(), allocate memory, inspect task ]
       │
       └──> put_task_struct(p)        ──> Decrements p->usage (Slab reclaims memory)
```
