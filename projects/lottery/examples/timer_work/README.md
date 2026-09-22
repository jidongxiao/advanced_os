# Timer-Driven Workqueue Kernel Module Demo

This example kernel module demonstrates how to combine **high-resolution timers (`hrtimer`)** and **workqueues (`work_struct`)** to create a **periodic, sleepable execution loop** in the Linux kernel.

## What This Module Demonstrates

* **Atomic to Process Context Hand-Off:** How to safely defer work from an atomic hardware interrupt context (`hrtimer`) to a sleepable process context (`kworker` thread).
* **Periodic High-Resolution Timers:** How to initialize, arm, and re-arm a recurring `hrtimer` using `ktime_t` intervals, including Linux kernel version compatibility handling (`hrtimer_setup` vs. `hrtimer_init`).
* **Asynchronous Deferred Work:** How to initialize a `work_struct` item with `INIT_WORK()` and trigger asynchronous process-context callback execution via `schedule_work()`.
* **Safe Module Teardown Sequence:** The correct cleanup order during module unload (`rmmod`)—canceling the timer first with `hrtimer_cancel()`, then flushing pending work with `cancel_work_sync()`—to prevent execution races and kernel memory faults.
