#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Educational Lab");
MODULE_DESCRIPTION("Safe multi-core atomic_t vs static int race demo");
MODULE_VERSION("2.2");

#define ITERATIONS 10000000

static int unsafe_counter = 0;
static atomic_t safe_counter = ATOMIC_INIT(0);
static atomic_t start_gate = ATOMIC_INIT(0);

static struct task_struct *thread1;
static struct task_struct *thread2;

static int worker_thread(void *data)
{
    int i;

    /* Spin until gate opens OR thread stop is requested */
    while (atomic_read(&start_gate) == 0 && !kthread_should_stop())
        cpu_relax();

    if (kthread_should_stop())
        return 0;

    for (i = 0; i < ITERATIONS; i++) {
        /* UNSAFE: Non-atomic increment (Data race under concurrency) */
        unsafe_counter++;

        /* SAFE: Atomic hardware-locked increment */
        atomic_inc(&safe_counter);
    }

    /* Wait for kthread_stop() signal before exiting */
    while (!kthread_should_stop())
        msleep(10);

    return 0;
}

static int __init atomic_demo_init(void)
{
    int cpu0 = 0;
    int cpu1 = num_online_cpus() > 1 ? 1 : 0;

    pr_info("[atomic_demo] Module loaded. Spawning synchronized kernel threads...\n");

    if (num_online_cpus() < 2) {
        pr_warn("[atomic_demo] System has only 1 CPU. Multi-core race requires >= 2 CPUs.\n");
    }

    /* Spawn threads pinned to CPUs */
    thread1 = kthread_create_on_cpu(worker_thread, NULL, cpu0, "atomic_worker/0");
    thread2 = kthread_create_on_cpu(worker_thread, NULL, cpu1, "atomic_worker/1");

    if (IS_ERR(thread1) || IS_ERR(thread2)) {
        pr_err("[atomic_demo] Failed to create kthreads\n");
        return -ENOMEM;
    }

    wake_up_process(thread1);
    wake_up_process(thread2);

    /* Signal threads to start counting concurrently */
    atomic_set(&start_gate, 1);

    return 0;
}

static void __exit atomic_demo_exit(void)
{
    /* Stop threads cleanly */
    if (thread1) kthread_stop(thread1);
    if (thread2) kthread_stop(thread2);

    pr_info("[atomic_demo] Expected Total (2 x %d): %d\n", ITERATIONS, ITERATIONS * 2);
    pr_info("[atomic_demo] -> Unsafe static int counter result: %d\n", unsafe_counter);
    pr_info("[atomic_demo] -> Safe atomic_t counter result:      %d\n", atomic_read(&safe_counter));
    pr_info("[atomic_demo] Module unloaded cleanly.\n");
}

module_init(atomic_demo_init);
module_exit(atomic_demo_exit);
