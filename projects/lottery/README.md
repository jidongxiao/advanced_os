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

Your load balancer runs periodically in process context (via `work_struct`). It must evaluate per-CPU queues and migrate tasks from the heaviest CPU core (`max_cpu`) to the lightest core (`min_cpu`) **only when both of the following criteria are met simultaneously**:

1. **Hysteresis Threshold (`max_tickets - min_tickets > 50`):** The ticket differential between the heaviest and lightest cores must strictly exceed **50 tickets**. This creates a deadband that avoids unnecessary context switches for negligible load differences. Examples: 

| **Scenario**              |  **Heaviest Core** |  **Lightest Core** | **Ticket Difference** | **Evaluation: `max_tickets - min_tickets > 50`** | **Verdict & Explanation**                                                                                                                                                                      |
| ------------------------- | -----------------: | -----------------: | --------------------: | ------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **60-Ticket Difference**  | **CPU 1:** 260 tix | **CPU 0:** 200 tix |        **60 tickets** | `260 - 200 > 50`<br><br>⟹ `60 > 50` (**TRUE**)   | **IMBALANCED — Migration Allowed**<br><br>The difference exceeds the threshold, so the load balancer can consider migrating a task.                                                          |
| **50-Ticket Difference**  | **CPU 1:** 250 tix | **CPU 0:** 200 tix |        **50 tickets** | `250 - 200 > 50`<br><br>⟹ `50 > 50` (**FALSE**)  | **BALANCED ENOUGH — Skip**<br><br>The difference is exactly 50, so the strict `>` condition prevents a migration. This avoids unnecessary context switches for a relatively small imbalance. |
| **40-Ticket Difference**  | **CPU 1:** 240 tix | **CPU 0:** 200 tix |        **40 tickets** | `240 - 200 > 50`<br><br>⟹ `40 > 50` (**FALSE**)  | **BALANCED ENOUGH — Skip**<br><br>The difference is below the threshold, so no migration is necessary.                                                                                       |

2. **Anti-Thrashing / Overshoot Prevention:** Before migrating a candidate task, verify that moving its ticket weight will **not** cause the destination core to become heavier than the source core (allowing a small 20-ticket tolerance buffer). This rule prevents the load balancer from making things worse when trying to balance two CPU cores. If you blindly move the first process you find from an overloaded core to an underloaded core, you might end up shifting too much weight, making the destination core heavier than the source core. This causes the two cores to ping-pong the task back and forth endlessly—a performance-destroying bug known as **thrashing**. Examples: 

| **Candidate Task**  | **Load Before Migration**                | **Load After Migration**                 | **Formula Evaluation: `dst_new <= src_new + 20`**                   | **Verdict & Explanation**                                                                                                          |
| ------------------- | ---------------------------------------- | ---------------------------------------- | ------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| **100-Ticket Task** | **CPU 1:** 320 tix<br>**CPU 0:** 200 tix | **CPU 1:** 220 tix<br>**CPU 0:** 300 tix | `(200 + 100) <= (320 - 100 + 20)`<br><br>⟹ `300 <= 240` (**FALSE**) | **NOT OK (Skip)**<br><br>Flips the imbalance (CPU 0 jumps from lightest to heaviest). Causes immediate ping-ponging (thrashing). |
| **30-Ticket Task**  | **CPU 1:** 320 tix<br>**CPU 0:** 200 tix | **CPU 1:** 290 tix<br>**CPU 0:** 230 tix | `(200 + 30) <= (320 - 30 + 20)`<br><br>⟹ `230 <= 310` (**TRUE**)    | **OK (Migrate)**<br><br>Brings both cores significantly closer to equilibrium without overshooting or flipping load ownership.   |


---

## Harness File Descriptions & Kernel Requirements

You are provided with `app.c`, `run_lottery.sh`, `Makefile`, and `lottery.h`. You must implement `lottery.c` to fulfill the expectations of these user-space components.

---

### 1. User Application Workload (`app.c`)

#### What `app.c` Does:

* **Opens the Character Device:** Opens `/dev/lottery` with `O_RDWR`.
* **Registers with the Kernel:** Fills a `struct lottery_struct` with its PID and ticket count, then invokes the `LOTTERY_REGISTER` `ioctl`.
* **Executes CPU Work:** Calculates the $n$-th Lucas number (`lucas(47)`) recursively to generate continuous CPU load.
* **Measures Wall-Clock Execution:** Tracks start and end times in milliseconds using `gettimeofday()`.
* **Unregisters from the Kernel:** Sends the `LOTTERY_UNREGISTER` `ioctl` upon completing its calculation and closes `/dev/lottery`.

#### Kernel Requirements for `app.c`:

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

- Queue Status Snapshots

```plaintext
[LOTTERY_STATUS] CPU0: <tix> tix (<N> tasks) | CPU1: <tix> tix (<N> tasks) | CPU2: <tix> tix (<N> tasks) | CPU3: <tix> tix (<N> tasks) |
```

- Migration Logs

```plaintext
[LOTTERY] Load Balance: Migrated PID <pid> (<tix> tix) CPU <src> -> CPU <dst>
```

## What You Need to Implement

You are provided with `app.c`, `run_lottery.sh`, `Makefile`, and `lottery.h`. You must write `lottery.c` to complete the kernel module.

## Running the Experiment

Run the automated evaluation script on your course VM (configured with 2 or 4 vCPUs):

```bash
sudo bash run_lottery.sh
```

## Expected Execution Log Output

When functioning correctly, your load balancer will demonstrate clear proportional execution scaling and queue equilibrium. Here is an example of a successful run on a 4-core VM:

```plaintext
test@test-vm:~/os/lottery$ sudo bash run_lottery.sh 
[sudo] password for test: 
make -C /lib/modules/7.0.0-test/build M=/home/test/os/lottery modules
make[1]: Entering directory '/home/test/linux-7.0'
make[2]: Entering directory '/home/test/os/lottery'
make[2]: Leaving directory '/home/test/os/lottery'
make[1]: Leaving directory '/home/test/linux-7.0'
=========================================
 Installing 'lottery' Kernel Module
=========================================

=========================================
 Starting Multi-Core Experiment (20 Tasks)
 Target: lucas(47) across 20 processes
 Ticket Distribution: 10 to 200 tickets
=========================================

Spawning 20 processes...
  [Process 01] PID: 4114 | Tickets: 10
  [Process 02] PID: 4116 | Tickets: 10
  [Process 03] PID: 4118 | Tickets: 10
  [Process 04] PID: 4120 | Tickets: 10
  [Process 05] PID: 4122 | Tickets: 10
  [Process 06] PID: 4124 | Tickets: 30
  [Process 07] PID: 4126 | Tickets: 30
  [Process 08] PID: 4128 | Tickets: 30
  [Process 09] PID: 4130 | Tickets: 30
  [Process 10] PID: 4132 | Tickets: 30
  [Process 11] PID: 4134 | Tickets: 60
  [Process 12] PID: 4136 | Tickets: 60
  [Process 13] PID: 4138 | Tickets: 60
  [Process 14] PID: 4140 | Tickets: 60
  [Process 15] PID: 4142 | Tickets: 60
  [Process 16] PID: 4144 | Tickets: 100
  [Process 17] PID: 4146 | Tickets: 100
  [Process 18] PID: 4148 | Tickets: 150
  [Process 19] PID: 4150 | Tickets: 150
  [Process 20] PID: 4152 | Tickets: 200

All 20 processes launched.
Waiting 3 seconds for initial process registration to settle...
--------------------------------------------------------
Capturing Steady-State Load Balancing Metrics...
--------------------------------------------------------
pid 4152, with 200 tickets: computing lucas(47) took 5.41 seconds.
pid 4150, with 150 tickets: computing lucas(47) took 7.11 seconds.
pid 4148, with 150 tickets: computing lucas(47) took 7.49 seconds.
pid 4144, with 100 tickets: computing lucas(47) took 9.88 seconds.
pid 4146, with 100 tickets: computing lucas(47) took 9.93 seconds.
pid 4138, with 60 tickets: computing lucas(47) took 12.17 seconds.
pid 4136, with 60 tickets: computing lucas(47) took 12.26 seconds.
pid 4140, with 60 tickets: computing lucas(47) took 12.60 seconds.
pid 4142, with 60 tickets: computing lucas(47) took 12.87 seconds.
pid 4134, with 60 tickets: computing lucas(47) took 12.99 seconds.
pid 4124, with 30 tickets: computing lucas(47) took 14.91 seconds.
pid 4132, with 30 tickets: computing lucas(47) took 16.44 seconds.
pid 4128, with 30 tickets: computing lucas(47) took 16.94 seconds.
pid 4130, with 30 tickets: computing lucas(47) took 17.58 seconds.
pid 4126, with 30 tickets: computing lucas(47) took 17.70 seconds.
pid 4118, with 10 tickets: computing lucas(47) took 19.14 seconds.
pid 4114, with 10 tickets: computing lucas(47) took 19.32 seconds.
pid 4122, with 10 tickets: computing lucas(47) took 19.42 seconds.
pid 4116, with 10 tickets: computing lucas(47) took 20.24 seconds.
pid 4120, with 10 tickets: computing lucas(47) took 20.47 seconds.

=========================================
 Total Steady-State Execution Time: 17.398378400s
=========================================

=========================================
 Steady-State Queue Load Snapshots
=========================================
[ 3142.478985] [LOTTERY_STATUS] CPU0: 270 tix (6 tasks) | CPU1: 320 tix (7 tasks) | CPU2: 310 tix (5 tasks) | CPU3: 300 tix (2 tasks) |
[ 3142.982947] [LOTTERY_STATUS] CPU0: 270 tix (6 tasks) | CPU1: 320 tix (7 tasks) | CPU2: 310 tix (5 tasks) | CPU3: 300 tix (2 tasks) |
[ 3143.478046] [LOTTERY_STATUS] CPU0: 270 tix (6 tasks) | CPU1: 320 tix (7 tasks) | CPU2: 310 tix (5 tasks) | CPU3: 300 tix (2 tasks) |
[ 3143.975285] [LOTTERY_STATUS] CPU0: 270 tix (6 tasks) | CPU1: 320 tix (7 tasks) | CPU2: 310 tix (5 tasks) | CPU3: 300 tix (2 tasks) |
[ 3144.476368] [LOTTERY_STATUS] CPU0: 270 tix (6 tasks) | CPU1: 310 tix (6 tasks) | CPU2: 310 tix (5 tasks) | CPU3: 110 tix (2 tasks) |
[ 3144.981963] [LOTTERY_STATUS] CPU0: 260 tix (5 tasks) | CPU1: 250 tix (4 tasks) | CPU2: 270 tix (3 tasks) | CPU3: 220 tix (7 tasks) |
[ 3145.475269] [LOTTERY_STATUS] CPU0: 260 tix (5 tasks) | CPU1: 250 tix (4 tasks) | CPU2: 270 tix (3 tasks) | CPU3: 220 tix (7 tasks) |
[ 3145.975968] [LOTTERY_STATUS] CPU0: 260 tix (5 tasks) | CPU1: 250 tix (4 tasks) | CPU2: 270 tix (3 tasks) | CPU3: 220 tix (7 tasks) |
[ 3146.485988] [LOTTERY_STATUS] CPU0: 220 tix (3 tasks) | CPU1: 220 tix (3 tasks) | CPU2: 190 tix (5 tasks) | CPU3: 220 tix (7 tasks) |
[ 3146.990915] [LOTTERY_STATUS] CPU0: 170 tix (5 tasks) | CPU1: 160 tix (2 tasks) | CPU2: 190 tix (5 tasks) | CPU3: 180 tix (5 tasks) |
[ 3147.475959] [LOTTERY_STATUS] CPU0: 170 tix (5 tasks) | CPU1: 160 tix (2 tasks) | CPU2: 190 tix (5 tasks) | CPU3: 180 tix (5 tasks) |
[ 3147.982943] [LOTTERY_STATUS] CPU0: 170 tix (5 tasks) | CPU1: 160 tix (2 tasks) | CPU2: 190 tix (5 tasks) | CPU3: 180 tix (5 tasks) |
[ 3148.483894] [LOTTERY_STATUS] CPU0: 170 tix (5 tasks) | CPU1: 160 tix (2 tasks) | CPU2: 190 tix (5 tasks) | CPU3: 180 tix (5 tasks) |
[ 3148.984364] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 120 tix (2 tasks) | CPU2: 130 tix (4 tasks) | CPU3: 140 tix (5 tasks) |
[ 3149.475957] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 120 tix (2 tasks) | CPU2: 130 tix (4 tasks) | CPU3: 140 tix (5 tasks) |
[ 3149.975956] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 120 tix (2 tasks) | CPU2: 130 tix (4 tasks) | CPU3: 140 tix (5 tasks) |
[ 3150.475210] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 120 tix (2 tasks) | CPU2: 130 tix (4 tasks) | CPU3: 140 tix (5 tasks) |
[ 3150.975955] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 120 tix (2 tasks) | CPU2: 130 tix (4 tasks) | CPU3: 140 tix (5 tasks) |
[ 3151.475960] [LOTTERY_STATUS] CPU0: 110 tix (4 tasks) | CPU1: 80 tix (4 tasks) | CPU2: 90 tix (2 tasks) | CPU3: 100 tix (3 tasks) |
[ 3151.975199] [LOTTERY_STATUS] CPU0: 50 tix (3 tasks) | CPU1: 80 tix (4 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 70 tix (2 tasks) |
[ 3152.475256] [LOTTERY_STATUS] CPU0: 50 tix (3 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3152.975231] [LOTTERY_STATUS] CPU0: 50 tix (3 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3153.475958] [LOTTERY_STATUS] CPU0: 50 tix (3 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3153.975211] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3154.480921] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3154.990842] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 60 tix (2 tasks) | CPU3: 20 tix (2 tasks) |
[ 3155.476007] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 70 tix (3 tasks) | CPU2: 30 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3155.975189] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 60 tix (2 tasks) | CPU2: 10 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3156.475952] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 60 tix (2 tasks) | CPU2: 10 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3156.975218] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 0 tix (0 tasks) | CPU2: 10 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3157.488814] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 0 tix (0 tasks) | CPU2: 10 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3157.975948] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 0 tix (0 tasks) | CPU2: 10 tix (1 tasks) | CPU3: 20 tix (2 tasks) |
[ 3158.653691] [LOTTERY_STATUS] CPU0: 20 tix (2 tasks) | CPU1: 0 tix (0 tasks) | CPU2: 0 tix (0 tasks) | CPU3: 0 tix (0 tasks) |
[ 3159.275255] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 0 tix (0 tasks) | CPU2: 0 tix (0 tasks) | CPU3: 0 tix (0 tasks) |

=========================================
 Steady-State Migration Events
=========================================
[ 3144.476011] [LOTTERY] Load Balance: Migrated PID 4116 (10 tix) CPU 1 -> CPU 3
[ 3144.525305] [LOTTERY] Load Balance: Migrated PID 4124 (30 tix) CPU 1 -> CPU 3
[ 3144.575301] [LOTTERY] Load Balance: Migrated PID 4118 (10 tix) CPU 2 -> CPU 3
[ 3144.637006] [LOTTERY] Load Balance: Migrated PID 4126 (30 tix) CPU 2 -> CPU 3
[ 3144.685334] [LOTTERY] Load Balance: Migrated PID 4132 (30 tix) CPU 1 -> CPU 3
[ 3144.727074] [LOTTERY] Load Balance: Migrated PID 4114 (10 tix) CPU 0 -> CPU 3
[ 3146.135046] [LOTTERY] Load Balance: Migrated PID 4122 (10 tix) CPU 0 -> CPU 2
[ 3146.175980] [LOTTERY] Load Balance: Migrated PID 4130 (30 tix) CPU 0 -> CPU 2
[ 3146.234984] [LOTTERY] Load Balance: Migrated PID 4128 (30 tix) CPU 1 -> CPU 2
[ 3146.533336] [LOTTERY] Load Balance: Migrated PID 4140 (60 tix) CPU 1 -> CPU 0
[ 3146.575317] [LOTTERY] Load Balance: Migrated PID 4116 (10 tix) CPU 3 -> CPU 0
[ 3146.626970] [LOTTERY] Load Balance: Migrated PID 4124 (30 tix) CPU 3 -> CPU 0
[ 3148.929116] [LOTTERY] Load Balance: Migrated PID 4134 (60 tix) CPU 2 -> CPU 3
[ 3148.983943] [LOTTERY] Load Balance: Migrated PID 4138 (60 tix) CPU 0 -> CPU 1
[ 3151.175280] [LOTTERY] Load Balance: Migrated PID 4118 (10 tix) CPU 3 -> CPU 1
[ 3151.225966] [LOTTERY] Load Balance: Migrated PID 4122 (10 tix) CPU 2 -> CPU 1
[ 3151.281923] [LOTTERY] Load Balance: Migrated PID 4126 (30 tix) CPU 3 -> CPU 1
[ 3151.327714] [LOTTERY] Load Balance: Migrated PID 4130 (30 tix) CPU 2 -> CPU 1
[ 3151.882707] [LOTTERY] Load Balance: Migrated PID 4132 (30 tix) CPU 3 -> CPU 2
[ 3152.058620] [LOTTERY] Load Balance: Migrated PID 4118 (10 tix) CPU 1 -> CPU 3
[ 3155.927066] [LOTTERY] Load Balance: Migrated PID 4122 (10 tix) CPU 1 -> CPU 2

Total Steady-State Migrations: 21

=========================================
 Cleaning Up Module
=========================================
Done.
```

> **Note:** You do **not** need to match the exact timestamps, PID numbers, or total migration counts shown in the example below. Your output will naturally vary depending on CPU speed and scheduling randomness. 
> 
> However, your implementation **must clearly demonstrate two key observations** in the execution log:
>
> 1. **Proportional Execution Scaling (Lottery Properties):** High-weight tasks (200, 150 tix) must complete significantly faster than medium-weight tasks (100, 60 tix), which in turn must finish before low-weight tasks (30, 10 tix).
> 2. **Dynamic Load Balancing:** Per-CPU ticket counts must stay relatively balanced across cores during steady state, and task migrations must trigger automatically whenever a core becomes underloaded (e.g., after a heavy task finishes) in accordance with the 2 migration criteria.

## Submission:

Submit the following files on Submitty:

1. lottery.c
2. README.txt - Include your test result (when running the run_lottery.sh script) in the README. If your program works correctly and produces the right results, you don't need to include anything else in the README. If your program does not work as expected, you can add some explanation on what works and what does not work and reflect on why your program does not work.

## Due Date

10/08/2026, 11:59pm.

## APIs

- Managing Per-CPU data structure is important in this assignment, and here is an [example kernel module](examples/percpu) showing how to manage per-CPU data structures containing doubly linked lists protected by spinlocks.

- Combining high-resolution timers (`hrtimer`) and workqueues (`INIT_WORK`) allows you to run sleepable tasks on a recurring schedule. Hardware timer interrupts run in atomic context, where sleeping, taking mutexes, or performing complex list operations is strictly prohibited. For this assignment, your load-balancing and task-migration logic must acquire per-CPU runqueue spinlocks and safely manipulate process lists every quantum—operations that require process context. Using the timer callback to enqueue a deferred work item enables a **periodic, sleepable execution loop** where your scheduler can safely evaluate multi-core ticket distribution, enforce hysteresis thresholds, and migrate tasks across cores. See the [timer workqueue example module](examples/timer_work) for a complete demonstration.

- Registering a miscellaneous character device (`misc_register`) and implementing `unlocked_ioctl` provides the user-space interface for your module. In this assignment, user processes need a way to interact with your kernel scheduler—such as registering themselves or adjusting their ticket allocations. A `miscdevice` automatically handles minor number allocation and creates a node under `/dev/` (e.g., `/dev/lottery`). Using `copy_from_user()` and `copy_to_user()` within your `unlocked_ioctl` handler ensures memory boundary safety when transferring scheduler commands and process metadata between user applications and the kernel. See the [misc ioctl example module](examples/misc_ioctl) for a complete demonstration.

- Controlling process execution state via POSIX signals (`send_sig_info`) allows your custom scheduler to preempt losing tasks and dispatch winning tasks without replacing the core Linux CPU scheduler. By issuing `send_sig_info(SIGSTOP, SEND_SIG_PRIV, task)`, the module pauses execution of non-winning processes for the current quantum. Issuing `send_sig_info(SIGCONT, SEND_SIG_PRIV, task)` resumes execution of the selected lottery winner. Resolving a process ID (PID) to a valid `struct task_struct` requires using `find_get_pid()` and `get_pid_task()`, making sure to manage reference counting (`put_pid`, `put_task_struct`) to avoid memory leaks. See the [signal control example module](examples/signal_control) for a complete demonstration.

- Executing cross-core load balancing requires physically migrating processes between CPU runqueues using set_cpus_allowed_ptr(). When your scheduler detects a ticket or task count imbalance exceeding your hysteresis threshold, setting the task's affinity mask via set_cpus_allowed_ptr(task_to_migrate, cpumask_of(min_cpu)) instructs the core Linux scheduler to migrate task_to_migrate to min_cpu. Remember that set_cpus_allowed_ptr() can sleep and must be invoked in process context (such as inside your deferred workqueue handler). See the [per-CPU migration example module](examples/percpu_migration) for a complete demonstration.

- In your lottery scheduler, you will traverse the process list. Call `rcu_read_lock()` before you traverse the process list, because the global process list is protected by Read-Copy-Update (RCU), and holding the RCU read lock guarantees pointer stability so that processes are not unlinked or modified mid-traversal (while strictly prohibiting sleeping or blocking inside the lock). When accessing a specific task struct across lock boundaries, outside RCU sections, or in deferred workqueues, call `get_task_struct(task)` because incrementing the atomic reference count pins the `task_struct` memory in the kernel slab heap, preventing Use-After-Free (UAF) kernel panics if the process terminates or exits concurrently. After accessing and finishing operations on the specific task struct, you must call `put_task_struct(task)`, which decrements the reference count and allows the kernel to safely reclaim its memory once all references are released. See the [task reference demonstration example module](examples/task_ref) for a complete demonstration.

- Correctly manipulating custom runqueues requires choosing between `list_add_tail()` and `list_move_tail()`. When registering a newly allocated task node in `LOTTERY_REGISTER`, use `list_add_tail()` to insert the fresh, unlinked node into the target CPU's queue tail. When performing load balancing or task migration across runqueues, use `list_move_tail()` to atomically detach the existing node from its source queue and append it to the destination queue. Using `list_add_tail()` on an already-linked node without unlinking it will corrupt double-linked list pointers and lead to kernel crashes. See the [list operations example module](examples/list_demo) for a complete demonstration.

- Safely inspecting and transferring tasks between two CPU runqueues during multi-core load balancing requires locking both queues simultaneously while preventing **AB-BA deadlocks** and complying with kernel **`lockdep`** rules. Always sort lock acquisition in strict ascending order of CPU ID (locking the lower CPU ID first regardless of which queue is source or destination) to eliminate circular lock dependencies. When acquiring the second spinlock of the same lock class, use `spin_lock_irqsave_nested(&dst->lock, flags2, SINGLE_DEPTH_NESTING)` to inform kernel deadlock validators that nesting is intentional and safe under your global hierarchy. Always release locks in exact reverse (LIFO) order using `spin_unlock_irqrestore()`. See the [double lock demonstration example module](examples/double_lock) for a complete demonstration.

- Generating random ticket selections in kernel space requires using kernel entropy interfaces rather than standard user-space C library calls. Using get_random_bytes(&rand_ticket, sizeof(rand_ticket)) from <linux/random.h> safely retrieves raw entropy from the kernel CSPRNG without sleeping or blocking atomic contexts. Applying modulo arithmetic bounded to total CPU queue tickets ((rand_ticket % q->total_tickets) + 1) produces a uniform 1-based winning ticket range [1, total_tickets] suitable for traversing runqueues and selecting winner tasks. See the [random number generator example module](examples/random_demo) for a complete demonstration.
