#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>   /* For LINUX_VERSION_CODE and KERNEL_VERSION */
#include <linux/hrtimer.h>   /* For hrtimer APIs, ktime_t, ktime_set() */
#include <linux/workqueue.h> /* For struct work_struct, INIT_WORK(), schedule_work() */
#include <linux/delay.h>     /* For msleep() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demonstration of HRTimer periodic tick triggering Process-Context Workqueue");

#define TIMER_INTERVAL_MS 500 /* 500 ms periodic tick */

static struct hrtimer demo_timer;
static struct work_struct demo_work;
static ktime_t kt_interval;

/*
 * Workqueue Handler Function (Runs in PROCESS CONTEXT via a kworker thread).
 *
 * Safe to sleep, allocate memory (GFP_KERNEL), take locks, or execute heavy loop logic.
 */
static void demo_work_func(struct work_struct *work)
{
    pr_info("timer_work_demo: Work handler running in process context (PID: %d, Comm: %s)\n",
            current->pid, current->comm);

    /* Simulate deferred work that takes time / sleeps */
    msleep(50);
}

/*
 * HRTimer Callback Handler (Runs in ATOMIC / INTERRUPT CONTEXT).
 *
 * MUST NOT SLEEP! CANNOT call msleep(), mutex_lock(), or memory allocation with GFP_KERNEL.
 * Instead, it delegates process-context work by calling schedule_work().
 */
static enum hrtimer_restart demo_timer_tick(struct hrtimer *timer)
{
    /* Offload deferred work to process context */
    schedule_work(&demo_work);

    /* Re-arm the timer for periodic execution */
    hrtimer_forward_now(timer, kt_interval);
    return HRTIMER_RESTART;
}

static int __init demo_init(void)
{
    pr_info("timer_work_demo: Initializing periodic timer and workqueue\n");

    /* 1. Initialize the deferred work item */
    INIT_WORK(&demo_work, demo_work_func);

    /* 2. Convert millisecond interval to scalar ktime_t nanoseconds */
    kt_interval = ktime_set(0, TIMER_INTERVAL_MS * 1000000);

    /* 3. Initialize/Setup HRTimer with Kernel Version Compatibility handling */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
    hrtimer_setup(&demo_timer, demo_timer_tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
    hrtimer_init(&demo_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    demo_timer.function = demo_timer_tick;
#endif

    /* 4. Start the relative high-resolution timer */
    hrtimer_start(&demo_timer, kt_interval, HRTIMER_MODE_REL);

    return 0;
}

static void __exit demo_exit(void)
{
    pr_info("timer_work_demo: Unloading module\n");

    /*
     * ALWAYS cancel the timer first to stop issuing new work, 
     * then flush/cancel any remaining pending or active work items!
     */
    hrtimer_cancel(&demo_timer);
    cancel_work_sync(&demo_work);

    pr_info("timer_work_demo: Module unloaded cleanly\n");
}

module_init(demo_init);
module_exit(demo_exit);
