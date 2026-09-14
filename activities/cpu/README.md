# Game: Absurd CPU Scheduler Design Challenge

## Objective

Work as a team to design an OS CPU scheduler with one absurd rule (e.g., processes are scheduled based on the alphabetical order of their current memory address).

Your job is to make it work technically using real CPU scheduling concepts (Task Struct / PCB, Runqueues, Time Slices, Context Switching, Preemption, Priority Decays) while keeping it fun and creative.

## How to Play

### Form Teams

Work in teams of 2–4 students. Come up with your own absurd rule that governs how the CPU scheduler selects and preempts tasks.

Choose one team member (or two) to be the presenter, and present your idea to the class.

### Design Your CPU Scheduler

Your team must answer these four questions:

What is your absurd rule?

How will you implement it? Use scheduling terminology and address these two specific aspects:

A. Kernel Data Structures: Any changes to the Process Control Block (task_struct), Runqueue (runqueue), Priority Levels, or Timer Interrupt structures?

B. Scheduler Operations: How are scheduler_tick(), pick_next_task(), context_switch(), and yield() / sleep() affected?

Name one scenario where your rule might actually be helpful.

What is one drawback of your rule?

### Present (2–3 Minutes Per Team)

Your chosen presenter will deliver a short, verbal-only pitch (no slides needed).

State your rule, explain the kernel mechanisms, and present your scenario and drawback.

### Voting & Awards

Class votes for Most Creative Absurd Scheduler Rule.

Extra Credit: +0.3% added to final course grade for the winning team.

### Examples

#### Example 1: Bribe-Based Priority Scheduler

Absurd Rule: A process can buy CPU time slices by transferring virtual memory pages or file descriptors back to the kernel.

Implementation Details:

Kernel Data Structures:

task_struct: Add a bribe_credits counter field.

runqueue: Replace the standard red-black tree with a bribe_priority_queue sorted by bribe_credits.

Scheduler Operations:

pick_next_task(): Always selects the process with the highest bribe_credits.

scheduler_tick(): Decrements bribe_credits on every clock interrupt. If bribe_credits == 0, the thread drops to the lowest priority runqueue.

yield() / system call: A new system call sys_bribe(void *addr) allows a process to surrender a physical memory page in exchange for 100 extra scheduling quantum ticks.

Helpful Scenario: High-priority real-time processes can dynamically "trade" unused heap space to guarantee sub-millisecond latency during sudden traffic spikes.

Drawback: Memory-starved tasks will experience absolute CPU starvation, eventually causing system-wide deadlocks.

#### Example 2: The "Decibel-Demanding" Screaming Scheduler

Absurd Rule: CPU clock frequency and time-slice allocation are directly scaled by how loudly the system administrator is screaming into the microphone. Silence results in immediate context-switching to idle.

Kernel Data Structures:

task_struct: Stores a decibel_budget decremented on every tick.

runqueue: Organized as a decibel-sorted priority min-heap.

pick_next_task(): Queries the sound card’s DMA buffer for real-time RMS audio levels. If input volume is below 85 dB, pick_next_task() refuses to return any user process and forces CPU core sleep states.

context_switch(): Higher pitch and volume increase the time-slice quantum from 1 ms to 100 ms.

Helpful Scenario: Prevents sysadmins from falling asleep during overnight emergency server outages.

Drawback: Causes permanent vocal cord damage for long-running batch jobs or kernel builds.

## Core Scheduler Functions Reference Guide

1. scheduler_tick()

When It Is Called: Automatically fired by the kernel on every hardware timer interrupt (typically 100 to 1000 times per second per CPU core, depending on CONFIG_HZ).

Kernel Purpose: It updates runtime statistics for the currently running process, accounting for how much CPU time it consumed during the last clock tick. It checks whether the current task has exhausted its allocated time slice (quantum). If the time slice is expired, it sets the NEED_RESCHED flag on the current task to request preemption.

2. pick_next_task()

When It Is Called: Called inside the core __schedule() execution loop whenever the CPU needs to switch to a new task (either because the current task yielded, blocked, or was preempted by scheduler_tick()).

Kernel Purpose: It iterates through scheduling classes (Deadline, Real-Time, Completely Fair Scheduler) and inspects the CPU’s runqueue (struct rq) to choose the highest-priority runnable task_struct to run next.

3. context_switch()

When It Is Called: Called at the very end of __schedule() after pick_next_task() has selected the target process.

Kernel Purpose: It performs the heavy lifting of swapping execution contexts from the old task_struct to the new task_struct. This involves two main actions:

Virtual Memory Switch (switch_mm): Swaps the page table pointer (CR3 register on x86) to point to the new process's address space.

Hardware Register Switch (switch_to): Swaps kernel stack pointers, program counter (IP/PC), and general-purpose CPU registers.

4. yield() / sleep()

When It Is Called:

yield(): Called explicitly by a user process (via sched_yield()) when it voluntarily surrenders the CPU to allow other threads to run.

sleep() / deactivate_task(): Called implicitly when a process blocks waiting for I/O, a semaphore, a timer, or a system call (e.g., read() from a disk file or socket).

Kernel Purpose: Removes the task_struct from the active runnable state in the runqueue and changes its state to TASK_INTERRUPTIBLE or TASK_UNINTERRUPTIBLE. The kernel then immediately calls schedule() to pick a different task.
