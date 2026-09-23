# Linux Kernel Module: `double_lock_demo`

A minimal, self-contained Linux kernel module demonstrating how to safely acquire multiple spinlocks simultaneously across per-CPU queues while preventing **AB-BA deadlocks** and complying with kernel **`lockdep` (Lock Dependency Validator)** rules.

---

## 1. Overview & Pedagogical Goals

In multi-core kernel subsystems (such as CPU schedulers), load balancing frequently requires inspecting and modifying two distinct per-CPU runqueues at the same time. Doing so requires holding spinlocks for both queues concurrently.

This module demonstrates two critical concurrency safety mechanisms:
1. **Ordered Lock Acquisition:** Sorting locks by CPU or structure ID to enforce a global lock hierarchy and eliminate circular wait conditions.
2. **Subclass Depth Annotations (`SINGLE_DEPTH_NESTING`):** Instructing the kernel validator that acquiring a second lock of the same class is intentional and safe.

---

## 2. Key Technical Concepts

### A. Why Lock Ordering Prevents Deadlocks
If CPU 0 tries to acquire Queue 0 -> Queue 1 while CPU 1 tries to acquire Queue 1 -> Queue 0, a circular dependency occurs (AB-BA Deadlock). By sorting the pointers by `cpu_id` before calling `spin_lock_irqsave`, both CPUs will attempt to lock Queue 0 first. The second CPU will simply wait for Queue 0 to become free rather than deadlocking.

### B. Why `SINGLE_DEPTH_NESTING` is Necessary
Both `queue_a.lock` and `queue_b.lock` share the same spinlock type/class. When the kernel is compiled with `CONFIG_PROVE_LOCKING=y` (standard on debug/Ubuntu kernels), locking two locks of the same class in one thread flags a potential recursive lock warning. `SINGLE_DEPTH_NESTING` informs the kernel's lock validator that taking this second lock is expected and safe due to our ordering guarantee.

### C. LIFO Unlocking Order
Locks acquired sequentially must be released in exact Last-In, First-Out (LIFO) stack order. Releasing `second` before `first` ensures that saved interrupt flags (`flags2`, `flags1`) are restored cleanly without leaving local CPU interrupts disabled prematurely.

---

## 3. Building the Module

### Compilation

```bash
make
```

---

## 4. Usage & Verification

### Scenario 1: Bidirectional Queue Transfers

1. Load the kernel module into the kernel.

```bash
sudo insmod double_lock_demo.ko
```

2. Inspect the kernel log (`dmesg`).

```bash
sudo dmesg | tail -n 5
```

3. Verify that the module executes transfers in both directions (CPU 0 -> CPU 1 and CPU 1 -> CPU 0) while maintaining the global acquisition hierarchy.

```bash
```

4. Unload the module cleanly.

```bash
sudo rmmod double_lock_demo
```

---

## 5. Code Architecture Summary

```text
  [ Transfer Request: Queue X <-> Queue Y ]
                    │
                    ▼
       Check: Is CPU(X) < CPU(Y)?
        ├── YES ──> First = X, Second = Y
        └── NO  ──> First = Y, Second = X
                    │
                    ▼
     spin_lock_irqsave(&First->lock, flags1)
     spin_lock_irqsave_nested(&Second->lock, flags2, SINGLE_DEPTH_NESTING)
                    │
                    ▼
          [ Critical Section ]
       (Safe multi-queue manipulation)
                    │
                    ▼
     spin_unlock_irqrestore(&Second->lock, flags2)
     spin_unlock_irqrestore(&First->lock, flags1)
```
