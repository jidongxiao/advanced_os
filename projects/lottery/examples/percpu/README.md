# Per-CPU Doubly Linked List Kernel Module Demo

This program contains a standalone, runnable Linux kernel module demonstrating how to manage **per-CPU data structures** containing **doubly linked lists** protected by **spinlocks**.

In symmetric multiprocessing (SMP) kernel development, per-CPU structures give every logical CPU core its own isolated instance of data. This design pattern eliminates cache-line bouncing and lock contention across cores, enabling high-performance parallel operation.

---

## What This Module Demonstrates

This module highlights five primary Linux kernel APIs and data structures:

### 1. Per-CPU Memory Allocation (`alloc_percpu` & `free_percpu`)
* **`alloc_percpu(type)`**: Dynamically allocates an array of `type` instances across all online CPUs in the system.
* **`free_percpu(ptr)`**: Releases the allocated per-CPU memory block during module exit (`rmmod`).

### 2. Per-CPU Pointer Resolution (`per_cpu_ptr`)
* **`per_cpu_ptr(ptr, cpu)`**: Converts the base `__percpu` pointer offset and a logical CPU ID (`cpu`) into a standard virtual memory address for that specific core.

### 3. Linux Kernel Doubly Linked List (`struct list_head`)
* **`INIT_LIST_HEAD(&head)`**: Initializes an empty list head by pointing its `next` and `prev` pointers to itself.
* **`list_add_tail(&item->node, &head)`**: Inserts a new node at the end of the per-CPU list.
* **`list_for_each_entry(pos, &head, member)`**: Iterates over list entries in a read-only loop.
* **`list_for_each_entry_safe(pos, tmp, &head, member)`**: Iterates over list entries safely when removing nodes or freeing memory inside the loop body.
* **`list_del(&item->node)`**: Unlinks a node from its parent doubly linked list.

### 4. Per-Core Synchronization (`spinlock_t`)
* **`spin_lock_init(&lock)`**: Prepares a spinlock for use.
* **`spin_lock(&lock)` / `spin_unlock(&lock)`**: Protects per-CPU list additions, traversals, and deletions from race conditions (e.g., concurrent process vs. timer or workqueue context).

### 5. Dynamic Kernel Memory (`kmalloc` & `kfree`)
* **`kmalloc(size, GFP_KERNEL)`**: Allocates dynamic kernel memory for individual list nodes.
* **`kfree(ptr)`**: Deallocates node memory during cleanup.

---

## File Structure

* `percpu_demo.c`: The kernel module source code implementing per-CPU list setup, item insertion, traversal, and cleanup.
* `Makefile`: Standard Kbuild Makefile to compile the out-of-tree module against your running kernel headers.

---

## How to Build, Load, and Inspect

### 1. Compile the Module

```bash
make
```

### 2. Insert the Module

Loading the module allocates per-CPU queues, populates two dummy items per core, and traverses all per-CPU lists:

```bash
sudo insmod percpu_demo.ko
```

### 3. Check Kernel Logs

Inspect dmesg to see how items were created and logged across each core:

```bash
dmesg | tail -n 25
```

Expected log output pattern:

```plaintext
[ 9856.935482] percpu_demo: Initializing per-CPU structures
[ 9856.935769] percpu_demo: CPU 0 initialized with 2 items
[ 9856.936048] percpu_demo: CPU 1 initialized with 2 items
[ 9856.936355] percpu_demo: CPU 2 initialized with 2 items
[ 9856.936678] percpu_demo: CPU 3 initialized with 2 items
[ 9856.936982] percpu_demo: CPU 0 has work_item ID 101
[ 9856.937273] percpu_demo: CPU 0 has work_item ID 102
[ 9856.937580] percpu_demo: CPU 1 has work_item ID 201
[ 9856.937874] percpu_demo: CPU 1 has work_item ID 202
[ 9856.938160] percpu_demo: CPU 2 has work_item ID 301
[ 9856.938454] percpu_demo: CPU 2 has work_item ID 302
[ 9856.938758] percpu_demo: CPU 3 has work_item ID 401
[ 9856.938982] percpu_demo: CPU 3 has work_item ID 402
```

### 4. Unload the Module

Unloading the module safely traverses each list, unlinks every node, frees dynamic memory, and releases the per-CPU array:

```plaintext
sudo rmmod percpu_demo
```

### 5. Clean Build Artifacts

```bash
make clean
```
