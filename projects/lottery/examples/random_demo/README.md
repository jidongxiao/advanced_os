# Linux Kernel Module: `random_demo`

A minimal, self-contained Linux kernel module demonstrating how to safely generate random numbers in kernel space using `get_random_bytes()` and scale them to a specific 1-based ticket range for lottery scheduling algorithms.

---

## 1. Overview

In user-space applications, developers typically rely on C library calls like `rand()` or `random()`. In kernel space, standard C library functions are unavailable. Kernel modules must interact directly with the kernel's **Cryptographically Secure Pseudo-Random Number Generator (CSPRNG)** via `<linux/random.h>`.

This module demonstrates:
1. Safely filling raw primitive variables with entropy using `get_random_bytes()`.
2. Mapping a random unsigned long integer into a bounded 1-based range `[1, N]` suitable for lottery scheduler ticket drawings.

---

## 2. Key Technical Concepts

### A. Kernel Entropy Generation (`get_random_bytes`)
The function `get_random_bytes(void *buf, int nbytes)` fills a memory buffer with raw bytes retrieved from the Linux kernel's entropy pool.
* **Header:** `#include <linux/random.h>`
* **Safety:** It works in atomic contexts (including interrupts or inside spinlocks) without sleeping.

### B. Range Bounding for Lottery Scheduling
To pick a winner in a lottery scheduler pool with `total_tickets` total tickets:
1. Extract a raw random integer `rand_ticket`.
2. Apply modulo arithmetic: `(rand_ticket % total_tickets)` produces a value in the range `[0, total_tickets - 1]`.
3. Add `1`: `(rand_ticket % total_tickets) + 1` shifts the range to `[1, total_tickets]`.

---

## 3. Building the Module

### Compilation
Run the build automation tool inside the module directory to compile the kernel object (`.ko`).

```bash
make
```

---

## 4. Usage & Verification

1. Load the kernel module into the kernel.

```bash
sudo insmod random_demo.ko
```

2. Inspect the kernel log (`dmesg`).

```bash
sudo dmesg | tail -n 5
```

3. Verify that the output prints a generated winning ticket within the range `[1, 100]`.

```bash
[89331.030216] [random_demo] Module loaded.
[89331.055142] [random_demo] Total Pool: 100 | Drawn Winning Ticket: 29
```

4. Unload the module cleanly.

```bash
sudo rmmod random_demo
```

---

## 5. Code Architecture Summary

```text
       [ Request Random Ticket ]
                  │
                  ▼
   get_random_bytes(&rand_ticket, sizeof(rand_ticket))
                  │
                  ▼
   rand_ticket = (rand_ticket % total_tickets) + 1
                  │
                  ▼
   [ Bounded Ticket Ready for Queue Traversal ]
```
