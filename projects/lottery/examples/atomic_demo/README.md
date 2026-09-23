# Linux Kernel Module: `atomic_demo`

A minimal, self-contained Linux kernel module demonstrating why `atomic_t` primitives are required instead of standard `static int` counters when tracking state, metrics, or throttling execution across concurrent workqueue threads and CPU cores.

---

## 1. Overview

In kernel space, multiple execution threads (such as workqueue handlers, timer interrupts, or system calls) can execute the same function concurrently across different CPU cores. Standard C integer increments (`counter++`) are not atomic—they expand into separate read, modify, and write CPU instructions that are vulnerable to **data races, lost updates, and CPU cache inconsistency**.

This module demonstrates:
1. The hardware-level difference between standard non-atomic integers and `atomic_t` types.
2. How to use `ATOMIC_INIT`, `atomic_inc`, `atomic_read`, and `atomic_inc_return` for lockless, thread-safe counter operations.

---

## 2. Key Technical Concepts

### A. Non-Atomic vs. Atomic Increments
* **Standard Integer (`static int`):** Performing `counter++` requires reading memory into a register, incrementing the register value, and writing it back to memory. When executed simultaneously on multiple cores, one core's write can overwrite another's, resulting in lost updates.
* **Atomic Type (`atomic_t`):** Operations like `atomic_inc()` map to atomic assembly instructions (e.g., `lock xadd` on x86 architectures). The bus or cache line is locked during execution, ensuring the read-modify-write cycle is completely indivisible across all cores.

### B. Cache Coherency and Memory Barriers
Standard primitive writes on one CPU core can remain in that core's local L1/L2 cache before being flushed to shared RAM, causing other cores to read stale counter values. `atomic_t` operations enforce implicit memory barriers, guaranteeing immediate cross-core visibility without requiring spinlocks or mutexes.

---

## 3. Building the Module

```bash
make
```

---

## 4. Usage & Verification

1. Load the kernel module into the kernel.

```bash
sudo insmod atomic_demo.ko
```

2. Wait for 1 minute, and then unload the module cleanly.

```bash
sudo rmmod atomic_demo
```
3. Observe how concurrent workqueue passes result in lost updates for the unsafe `static int` counter while the `atomic_t` counter yields exact, thread-safe totals.

```bash
[  210.710468] [atomic_demo] Module loaded. Spawning synchronized kernel threads...
[  290.658931] [atomic_demo] Expected Total (2 x 10000000): 20000000
[  290.659275] [atomic_demo] -> Unsafe static int counter result: 15818812
[  290.659604] [atomic_demo] -> Safe atomic_t counter result:      20000000
[  290.659975] [atomic_demo] Module unloaded cleanly.
```

---

## 5. Code Architecture Summary

```text
       [ Concurrent Execution across Core 0 and Core 1 ]
                           │
       ┌───────────────────┴───────────────────┐
       ▼                                       ▼
  Non-Atomic: counter++                   Atomic: atomic_inc()
  (Read-Modify-Write)                     (Hardware Bus Lock)
       │                                       │
  Vulnerable to Data Races               Thread-Safe & Lockless
       │                                       │
       ▼                                       ▼
  [ Lost Updates / Corrupted State ]      [ Exact Atomic Counter ]
