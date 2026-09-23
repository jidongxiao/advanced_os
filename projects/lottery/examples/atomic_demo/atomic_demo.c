#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demo comparing non-atomic static int vs atomic_t counters in workqueues");
MODULE_VERSION("1.0");

/* Work items to simulate concurrent execution */
static struct work_struct work1;
static struct work_struct work2;

/* Demonstration work handler called concurrently on multiple workqueue threads */
static void worker_function(struct work_struct *work)
{
    /* Static local counters initialized once */
    static int unsafe_counter = 0;
    static atomic_t safe_counter = ATOMIC_INIT(0);
    
    int i;
    int current_safe_val;

    for (i = 0; i < 100000; i++) {
        /* UNSAFE: Non-atomic read-modify-write (Data race under concurrency) */
        unsafe_counter++;

        /* SAFE: Lockless, hardware-enforced atomic increment */
        atomic_inc(&safe_counter);
    }

    current_safe_val = atomic_read(&safe_counter);

    pr_info("[atomic_demo] Thread finished iteration pass.\n");
    pr_info("[atomic_demo] -> Unsafe static int counter: %d\n", unsafe_counter);
    pr_info("[atomic_demo] -> Safe atomic_t counter:      %d\n", current_safe_val);
}

static int __init atomic_demo_init(void)
{
    pr_info("[atomic_demo] Module loaded. Dispatching concurrent work items...\n");

    INIT_WORK(&work1, worker_function);
    INIT_WORK(&work2, worker_function);

    /* Queue both work items to system workqueue to run concurrently */
    schedule_work(&work1);
    schedule_work(&work2);

    return 0;
}

static void __exit atomic_demo_exit(void)
{
    /* Flush pending work to prevent module unload race conditions */
    cancel_work_sync(&work1);
    cancel_work_sync(&work2);

    pr_info("[atomic_demo] Module unloaded cleanly.\n");
}

module_init(atomic_demo_init);
module_exit(atomic_demo_exit);
