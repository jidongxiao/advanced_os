# Cross-CPU Task Migration Kernel Module Demo

This example kernel module demonstrates how a custom kernel scheduler enforces task migration across online CPU cores using `set_cpus_allowed_ptr()`.

## What This Module Demonstrates

* **CPU Affinity Manipulation:** Restricting a process's affinity mask using `set_cpus_allowed_ptr()` and `cpumask_of()` to force the core Linux scheduler to migrate the thread to a target CPU.
* **Online CPU Mask Traversal:** Safely querying active system cores using `num_online_cpus()`, `cpu_online_mask`, and `cpumask_next()`.
* **Safe Task Reference Management:** Resolving PID integers to `struct task_struct *` via `find_get_pid()` and `get_pid_task()`, with reference cleanup (`put_pid()`, `put_task_struct()`).

## Building and Testing

### 1. Build the Module
Compile the module against your running kernel headers.

```bash
make
```

### 2. Start a Target Application
Run a CPU-bound process or background loop in a separate terminal and note its process ID (PID).

```bash
nohup python3 -c "while True: pass" > /dev/null 2>&1 &
```

Note the PID (e.g., 12345).

### 3. Load Module with `target_pid`
Insert the module into the kernel, providing the PID of the target application via the required module parameter.

```bash
sudo insmod percpu_migration_demo.ko target_pid=12345h
```

### 4. Observe Migration in Action
Monitor kernel logging to confirm that the process is moved across cores every 5 seconds. You can also inspect the active processor core column (`PSR`) in process management tools.

```bash
sudo dmesg -w
```

You should observe the migration like this:

```bash
[15595.797319] percpu_migration_demo: Migrated PID 8194 (python3) from CPU 0 -> CPU 1
[15600.801519] percpu_migration_demo: Migrated PID 8194 (python3) from CPU 1 -> CPU 2
[15605.820985] percpu_migration_demo: Migrated PID 8194 (python3) from CPU 2 -> CPU 3
[15610.796247] percpu_migration_demo: Migrated PID 8194 (python3) from CPU 3 -> CPU 0
[15615.797473] percpu_migration_demo: Migrated PID 8194 (python3) from CPU 0 -> CPU 1
```

### 5. Cleanup
Unload the module to restore default CPU affinity across all online cores, terminate the test process, and clean up build artifacts.

```bash
sudo rmmod percpu_migration_demo
kill -9 12345
make clean
```
