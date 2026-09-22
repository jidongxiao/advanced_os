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

## Migration Criteria & Load Balancing Rules

Your load balancer runs periodically in process context (via `work_struct`). It must evaluate per-CPU queues and migrate tasks from the heaviest CPU core (`max_cpu`) to the lightest core (`min_cpu`) **only when all three of the following criteria are met simultaneously**:

1. **Distinct Core Selection (`max_cpu != min_cpu`):** The target destination core must differ from the source core.
2. **Hysteresis Threshold (`max_tickets - min_tickets > 50`):** The ticket differential between the heaviest and lightest cores must strictly exceed **50 tickets**. This creates a deadband that avoids unnecessary context switches for negligible load differences. Examples: 

| **Scenario**              |  **Heaviest Core** |  **Lightest Core** | **Ticket Difference** | **Evaluation: `max_tickets - min_tickets > 50`** | **Verdict & Explanation**                                                                                                                                                                      |
| ------------------------- | -----------------: | -----------------: | --------------------: | ------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **60-Ticket Difference**  | **CPU 1:** 260 tix | **CPU 0:** 200 tix |        **60 tickets** | `260 - 200 > 50`<br><br>⟹ `60 > 50` (**TRUE**)   | **IMBALANCED — Migration Allowed**<br><br>The difference exceeds the threshold, so the load balancer can consider migrating a task.                                                          |
| **50-Ticket Difference**  | **CPU 1:** 250 tix | **CPU 0:** 200 tix |        **50 tickets** | `250 - 200 > 50`<br><br>⟹ `50 > 50` (**FALSE**)  | **BALANCED ENOUGH — Skip**<br><br>The difference is exactly 50, so the strict `>` condition prevents a migration. This avoids unnecessary context switches for a relatively small imbalance. |
| **40-Ticket Difference**  | **CPU 1:** 240 tix | **CPU 0:** 200 tix |        **40 tickets** | `240 - 200 > 50`<br><br>⟹ `40 > 50` (**FALSE**)  | **BALANCED ENOUGH — Skip**<br><br>The difference is below the threshold, so no migration is necessary.                                                                                       |
| **100-Ticket Difference** | **CPU 1:** 300 tix | **CPU 0:** 200 tix |       **100 tickets** | `300 - 200 > 50`<br><br>⟹ `100 > 50` (**TRUE**)  | **IMBALANCED — Migration Allowed**<br><br>The large difference clearly exceeds the hysteresis threshold, so migration should be considered.                                                  |

3. **Anti-Thrashing / Overshoot Prevention:** Before migrating a candidate task, verify that moving its ticket weight will **not** cause the destination core to become heavier than the source core (allowing a small 20-ticket tolerance buffer). This rule prevents the load balancer from making things worse when trying to balance two CPU cores. If you blindly move the first process you find from an overloaded core to an underloaded core, you might end up shifting too much weight, making the destination core heavier than the source core. This causes the two cores to ping-pong the task back and forth endlessly—a performance-destroying bug known as **thrashing**. Examples: 

| **Candidate Task**  | **Load Before Migration**                | **Load After Migration**                 | **Formula Evaluation: `dst_new <= src_new + 20`**                   | **Verdict & Explanation**                                                                                                          |
| ------------------- | ---------------------------------------- | ---------------------------------------- | ------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| **100-Ticket Task** | **CPU 1:** 320 tix<br>**CPU 0:** 200 tix | **CPU 1:** 220 tix<br>**CPU 0:** 300 tix | `(200 + 100) <= (320 - 100 + 20)`<br><br>⟹ `300 <= 240` (**FALSE**) | **NOT OK (Skip)**<br><br>Flips the imbalance (CPU 0 jumps from lightest to heaviest). Causes immediate ping-ponging (thrashing). |
| **30-Ticket Task**  | **CPU 1:** 320 tix<br>**CPU 0:** 200 tix | **CPU 1:** 290 tix<br>**CPU 0:** 230 tix | `(200 + 30) <= (320 - 30 + 20)`<br><br>⟹ `230 <= 310` (**TRUE**)    | **OK (Migrate)**<br><br>Brings both cores significantly closer to equilibrium without overshooting or flipping load ownership.   |


---

## Harness File Descriptions & Kernel Requirements

You are provided with `app.c`, `run_lottery.sh`, `Makefile`, and `lottery.h`. You must implement `lottery.c` to fulfill the expectations of these user-space components.

### What `app.c` Does:

* **Opens the Character Device:** Opens `/dev/lottery` with `O_RDWR`.
* **Registers with the Kernel:** Fills a `struct lottery_struct` with its PID and ticket count, then invokes the `LOTTERY_REGISTER` `ioctl`.
* **Executes CPU Work:** Calculates the $n$-th Lucas number (`lucas(47)`) recursively to generate continuous CPU load.
* **Measures Wall-Clock Execution:** Tracks start and end times in milliseconds using `gettimeofday()`.
* **Unregisters from the Kernel:** Sends the `LOTTERY_UNREGISTER` `ioctl` upon completing its calculation and closes `/dev/lottery`.

### Kernel Requirements for `app.c`:

* **Character Device (`/dev/lottery`):** Must be created as a character misc device upon module loading (`insmod`).
* **`LOTTERY_REGISTER` ioctl:**
  * Receives `struct lottery_struct { unsigned long pid; unsigned long tickets; }`.
  * Creates a new `struct lottery_task` entry.
  * Places the process onto the lightest per-CPU queue (`total_tickets` basis).
  * Binds the process to that core using `set_cpus_allowed_ptr()`.
* **`LOTTERY_UNREGISTER` ioctl:**
  * Removes the process from its assigned per-CPU queue.
  * Deducts its tickets from `total_tickets` and decrements `task_count`.
  * Frees the associated `struct lottery_task` memory.

---

### 2. Automated Test Harness (`run_lottery.sh`)

`run_lottery.sh` automates the entire multi-core experiment lifecycle and parses kernel logs for grading metrics.

#### Execution Pipeline:
1. **Privilege & Build Check:** Verifies root privileges (`sudo`), compiles the code via `make`, and loads `lottery.ko`.
2. **Device Permission Grant:** Sets device permissions (`chmod 666 /dev/lottery`) so worker tasks can issue `ioctl` calls.
3. **Task Spawning:** Launches 20 parallel worker processes in the background with ticket weights ranging from 10 to 200:
   * **10 tickets:** 5 tasks
   * **30 tickets:** 5 tasks
   * **60 tickets:** 5 tasks
   * **100 tickets:** 2 tasks
   * **150 tickets:** 2 tasks
   * **200 tickets:** 1 task
4. **Registration Settle Phase:** Pauses for 3 seconds (`sleep 3`) to allow all 20 processes to register and bind to initial cores, then flushes pre-existing kernel logs (`dmesg -c`).
5. **Steady-State Monitoring:** Uses `wait` to block until all 20 background processes complete, recording total wall-clock execution time.
6. **Log Extraction & Cleanup:** Parses kernel ring buffer output for status snapshots and migration events before unloading `lottery.ko`.

#### Kernel Log Formatting Requirements for `run_lottery.sh`:
To ensure the test script can extract evaluation metrics, your kernel module must emit `pr_info()` logs formatted as follows:

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
```

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


