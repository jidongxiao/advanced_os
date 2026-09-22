# Programming Assignment: Multi-Core Proportional-Share Lottery Scheduler

## Overview

In this assignment, you will implement a **Multi-Core Lottery Scheduler** as a Linux kernel module (`lottery.ko`). 

Lottery scheduling is a randomized, proportional-share resource allocation algorithm. Each process is assigned a specific number of **tickets**, and CPU time is allocated probabilistically based on ticket ratios. To scale lottery scheduling across multiple CPU cores without global lock contention, you will maintain **per-CPU runqueues** and implement a **dynamic load-balancing mechanism** to maintain work equilibrium across all online cores.

---

## Objectives

1. Maintain per-CPU task lists and ticket totals using kernel per-CPU data structures (`alloc_percpu`).
2. Implement proportional-share winner selection using kernel random number generation (`get_random_bytes`).
3. Enforce context switching via kernel signaling (`SIGSTOP` / `SIGCONT`).
4. Design a deadlock-free, thrashing-resistant **load balancer** to migrate tasks between CPU cores.
5. Provide a system interface via a character misc device (`/dev/lottery`) using `ioctl`.

---

## System Architecture & Flow

User Space                     Kernel Space (/dev/lottery)
+------------+   ioctl()    +------------------------------------+
|  app.c     | -----------> | lottery_ioctl()                   |
|  (Workers) |              |  - LOTTERY_REGISTER                |
+------------+              |  - LOTTERY_UNREGISTER              |
+------------------------------------+
|
Per-CPU Queues
+------------------+   +------------------+
| CPU 0 Queue      |   | CPU 1 Queue      |
| - total_tickets  |   | - total_tickets  |
| - task_list      |   | - task_list      |
+------------------+   +------------------+
^                 ^
|   Workqueue     |
+-----------------+
balance_cpu_loads()
lottery_sched_work_func()

---

## Migration Criteria & Load Balancing Rules

Your load balancer runs periodically in process context (via `work_struct`). It must evaluate per-CPU queues and migrate tasks from the heaviest CPU core (`max_cpu`) to the lightest core (`min_cpu`) **only when all four of the following criteria are met simultaneously**:

1. **Distinct Core Selection (`max_cpu != min_cpu`):** The target destination core must differ from the source core.
2. **Hysteresis Threshold (`max_tickets - min_tickets > 50`):** The ticket differential between the heaviest and lightest cores must strictly exceed **50 tickets**. This creates a deadband that avoids unnecessary context switches for negligible load differences.
3. **Anti-Thrashing / Overshoot Prevention:**
   $$\text{dst\_tickets} + \text{task\_tickets} \le \text{src\_tickets} - \text{task\_tickets} + 20$$
   You must iterate through the source queue to find a candidate task that does not cause the destination queue to overshoot the source queue (allowing a 20-ticket tolerance margin).
4. **Valid Task Pointer:** A candidate task node must exist, and its corresponding `struct task_struct *` must be pinned safely with `get_task_struct()`.

---

## Important Kernel Implementation Pitfalls

> ⚠️ **CRITICAL WARNINGS:**
> 
> 1. **Never call sleeping functions while holding a spinlock!**
>    Functions like `set_cpus_allowed_ptr()` sleep while waiting for task migration completion. Calling them inside a `spin_lock_irq()` critical section will cause an immediate `BUG: scheduling while atomic` kernel panic. You **must drop all spinlocks** before invoking `set_cpus_allowed_ptr()`.
>
> 2. **Prevent AB-BA Lock Inversion Deadlocks:**
>    When locking two per-CPU queues during migration, always acquire locks in ascending order of CPU indices:
>    ```c
>    if (max_cpu < min_cpu) {
>        spin_lock_irqsave(&src_q->lock, flags1);
>        spin_lock_irqsave_nested(&dst_q->lock, flags2, SINGLE_DEPTH_NESTING);
>    } else {
>        spin_lock_irqsave(&dst_q->lock, flags1);
>        spin_lock_irqsave_nested(&src_q->lock, flags2, SINGLE_DEPTH_NESTING);
>    }
>    ```
>
> 3. **Pin Task References Across Migration:**
>    To prevent a Use-After-Free (UAF) if a task unregisters while being migrated, acquire a reference using `get_task_struct(p)` while holding the queue lock, and drop it with `put_task_struct(p)` after `set_cpus_allowed_ptr()` finishes.

---

## What You Need to Implement

You are provided with `app.c`, `run_lottery.sh`, `Makefile`, and `lottery.h`. You must write `lottery.c` to complete the kernel module.

### Data Structures (`lottery.c`)

```c
struct lottery_task {
    pid_t pid;
    unsigned long tickets;
    struct task_struct *task;
    int assigned_cpu;
    u64 total_runtime_ms;
    struct list_head node;
};

struct lottery_cpu_queue {
    spinlock_t lock;
    struct list_head tasks;
    unsigned long total_tickets;
    unsigned int task_count;
};

# Required Functions

static int __init lottery_init(void): Allocate per-CPU structures, set up the High-Resolution timer (hrtimer), initialize the work item (INIT_WORK), and register the misc device /dev/lottery.

static void __exit lottery_exit(void): Cancel timers, flush workqueues, restore all queued processes (SIGCONT), free per-CPU memory, and unregister the misc device.

static long lottery_ioctl(...): Handle LOTTERY_REGISTER (place on the lightest CPU queue, bind affinity) and LOTTERY_UNREGISTER (remove from queue, unpin task).

static void balance_cpu_loads(void): Evaluate queue tickets across all cores and migrate candidate tasks according to the 4 migration criteria.

static void lottery_sched_work_func(struct work_struct *work): Invoke balance_cpu_loads(), pick winners via pseudo-random lottery selection, update runtime statistics, and issue signals (SIGCONT for winner, SIGSTOP for losers).

static enum hrtimer_restart lottery_sched_tick(struct hrtimer *timer): Periodic 50ms tick that schedules sched_work onto the system workqueue.

# Running the Experiment

Run the automated evaluation script on your course VM (configured with 2 or 4 vCPUs):

```bash
sudo bash run_lottery.sh
```

# Expected Execution Log Output

When functioning correctly, your load balancer will demonstrate clear proportional execution scaling and queue equilibrium:


