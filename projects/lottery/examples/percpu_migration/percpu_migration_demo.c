#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>      /* Required for LINUX_VERSION_CODE and KERNEL_VERSION */
#include <linux/sched.h>        /* For struct task_struct, get_pid_task(), task_cpu() */
#include <linux/sched/task.h>   /* For set_cpus_allowed_ptr() */
#include <linux/cpumask.h>      /* For cpumask_of(), num_online_cpus(), cpu_online_mask */
#include <linux/pid.h>          /* For find_get_pid() */
#include <linux/hrtimer.h>      /* For hrtimer APIs */
#include <linux/workqueue.h>    /* For struct work_struct, INIT_WORK(), schedule_work() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demonstration of Cross-CPU Task Migration using set_cpus_allowed_ptr");

/* Module parameter: PID of the target user process to migrate */
static int target_pid = -1;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "PID of the target process to migrate between CPU cores");

static struct hrtimer migrate_timer;
static struct work_struct migrate_work;
static ktime_t kt_interval;
static int current_target_cpu = 0;

/*
 * Workqueue Handler (Process Context)
 * Resolves target_pid to task_struct, selects the next online CPU core,
 * and migrates the task using set_cpus_allowed_ptr().
 */
static void migrate_work_func(struct work_struct *work)
{
    struct pid *pid_struct;
    struct task_struct *task;
    int old_cpu, next_cpu, ret;

    if (target_pid <= 0)
        return;

    /* 1. Resolve struct pid from PID integer */
    pid_struct = find_get_pid(target_pid);
    if (!pid_struct) {
        pr_err("percpu_migration_demo: Could not find pid_struct for PID %d\n", target_pid);
        return;
    }

    /* 2. Resolve struct task_struct from pid_struct */
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct); /* Drop reference to pid_struct */

    if (!task) {
        pr_err("percpu_migration_demo: Target task for PID %d no longer exists\n", target_pid);
        return;
    }

    /* 3. Record current CPU execution core BEFORE requesting migration */
    old_cpu = task_cpu(task);

    /* 4. Determine the next online CPU core to target */
    next_cpu = cpumask_next(current_target_cpu, cpu_online_mask);
    if (next_cpu >= nr_cpu_ids)
        next_cpu = cpumask_first(cpu_online_mask);

    /* 5. Enforce CPU affinity constraint to trigger kernel task migration */
    ret = set_cpus_allowed_ptr(task, cpumask_of(next_cpu));
    if (ret == 0) {
        pr_info("percpu_migration_demo: Migrated PID %d (%s) from CPU %d -> CPU %d\n",
                target_pid, task->comm, old_cpu, next_cpu);
        current_target_cpu = next_cpu;
    } else {
        pr_err("percpu_migration_demo: Failed to migrate PID %d to CPU %d (err=%d)\n",
               target_pid, next_cpu, ret);
    }

    /* 6. Drop reference to task_struct */
    put_task_struct(task);
}

/*
 * Atomic Timer Callback: Enqueues migration work in process context.
 */
static enum hrtimer_restart migrate_timer_tick(struct hrtimer *timer)
{
    schedule_work(&migrate_work);
    hrtimer_forward_now(timer, kt_interval);
    return HRTIMER_RESTART;
}

static int __init demo_init(void)
{
    if (target_pid <= 0) {
        pr_err("percpu_migration_demo: Please provide a valid target_pid parameter!\n");
        pr_err("Usage: insmod percpu_migration_demo.ko target_pid=<PID>\n");
        return -EINVAL;
    }

    if (num_online_cpus() < 2) {
        pr_warn("percpu_migration_demo: System has only 1 online CPU core. Migration requires >= 2 CPUs!\n");
    }

    pr_info("percpu_migration_demo: Initialized migration demo for PID %d\n", target_pid);

    INIT_WORK(&migrate_work, migrate_work_func);

    /* Migrate target task every 5 seconds */
    kt_interval = ktime_set(5, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
    hrtimer_setup(&migrate_timer, migrate_timer_tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
    hrtimer_init(&migrate_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    migrate_timer.function = migrate_timer_tick;
#endif

    hrtimer_start(&migrate_timer, kt_interval, HRTIMER_MODE_REL);

    return 0;
}

static void __exit demo_exit(void)
{
    struct pid *pid_struct;
    struct task_struct *task;

    hrtimer_cancel(&migrate_timer);
    cancel_work_sync(&migrate_work);

    /* Restore target process CPU affinity mask to all online CPUs on cleanup */
    if (target_pid > 0) {
        pid_struct = find_get_pid(target_pid);
        if (pid_struct) {
            task = get_pid_task(pid_struct, PIDTYPE_PID);
            put_pid(pid_struct);
            if (task) {
                set_cpus_allowed_ptr(task, cpu_online_mask);
                pr_info("percpu_migration_demo: Restored full CPU affinity for PID %d\n", target_pid);
                put_task_struct(task);
            }
        }
    }

    pr_info("percpu_migration_demo: Module unloaded cleanly\n");
}

module_init(demo_init);
module_exit(demo_exit);
