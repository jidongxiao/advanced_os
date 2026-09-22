#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>     /* For struct task_struct, get_pid_task() */
#include <linux/sched/signal.h> /* For send_sig_info(), SEND_SIG_PRIV */
#include <linux/pid.h>       /* For find_get_pid() */
#include <linux/hrtimer.h>   /* For hrtimer APIs */
#include <linux/workqueue.h> /* For struct work_struct, INIT_WORK(), schedule_work() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demonstration of Kernel Process Control using SIGSTOP and SIGCONT");

/* Module parameter: PID of the target user process to control */
static int target_pid = -1;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "PID of the target process to toggle between STOP and CONT");

static struct hrtimer toggle_timer;
static struct work_struct toggle_work;
static ktime_t kt_interval;
static bool is_stopped = false;

/*
 * Workqueue Handler (Process Context)
 * Fetches the task_struct by PID and alternates between pausing (SIGSTOP)
 * and resuming (SIGCONT) the target process.
 */
static void toggle_work_func(struct work_struct *work)
{
    struct pid *pid_struct;
    struct task_struct *task;

    if (target_pid <= 0)
        return;

    /* 1. Resolve struct pid from PID integer */
    pid_struct = find_get_pid(target_pid);
    if (!pid_struct) {
        pr_err("signal_control_demo: Could not find pid_struct for PID %d\n", target_pid);
        return;
    }

    /* 2. Resolve struct task_struct from pid_struct */
    task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct); /* Drop reference to pid_struct after getting task */

    if (!task) {
        pr_err("signal_control_demo: Target task for PID %d no longer exists\n", target_pid);
        return;
    }

    /* 3. Send kernel-privileged signal to toggle execution state */
    if (is_stopped) {
        pr_info("signal_control_demo: Sending SIGCONT to PID %d (%s)\n", target_pid, task->comm);
        send_sig_info(SIGCONT, SEND_SIG_PRIV, task);
        is_stopped = false;
    } else {
        pr_info("signal_control_demo: Sending SIGSTOP to PID %d (%s)\n", target_pid, task->comm);
        send_sig_info(SIGSTOP, SEND_SIG_PRIV, task);
        is_stopped = true;
    }

    /* Drop reference to task_struct when finished */
    put_task_struct(task);
}

/*
 * Atomic Timer Callback: Enqueues work to send signals in process context.
 */
static enum hrtimer_restart toggle_timer_tick(struct hrtimer *timer)
{
    schedule_work(&toggle_work);
    hrtimer_forward_now(timer, kt_interval);
    return HRTIMER_RESTART;
}

static int __init demo_init(void)
{
    if (target_pid <= 0) {
        pr_err("signal_control_demo: Please provide a valid target_pid parameter!\n");
        pr_err("Usage: insmod signal_control_demo.ko target_pid=<PID>\n");
        return -EINVAL;
    }

    pr_info("signal_control_demo: Initialized targeting PID %d\n", target_pid);

    INIT_WORK(&toggle_work, toggle_work_func);

    /* Toggle state every 1 second (1000 ms) */
    kt_interval = ktime_set(1, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
    hrtimer_setup(&toggle_timer, toggle_timer_tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
    hrtimer_init(&toggle_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    toggle_timer.function = toggle_timer_tick;
#endif

    hrtimer_start(&toggle_timer, kt_interval, HRTIMER_MODE_REL);

    return 0;
}

static void __exit demo_exit(void)
{
    struct pid *pid_struct;
    struct task_struct *task;

    hrtimer_cancel(&toggle_timer);
    cancel_work_sync(&toggle_work);

    /* Ensure target process is left running (SIGCONT) upon unloading */
    if (target_pid > 0 && is_stopped) {
        pid_struct = find_get_pid(target_pid);
        if (pid_struct) {
            task = get_pid_task(pid_struct, PIDTYPE_PID);
            put_pid(pid_struct);
            if (task) {
                pr_info("signal_control_demo: Clean exit - resuming PID %d\n", target_pid);
                send_sig_info(SIGCONT, SEND_SIG_PRIV, task);
                put_task_struct(task);
            }
        }
    }

    pr_info("signal_control_demo: Module unloaded cleanly\n");
}

module_init(demo_init);
module_exit(demo_exit);
