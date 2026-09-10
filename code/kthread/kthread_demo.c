#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("kthread example");

static struct task_struct *my_thread;

static int thread_fn(void *data)
{
    int counter = 0;

    // Used inside the thread function to check if it should exit gracefully.
    // This function returns true (non-zero) if kthread_stop() has been called and the thread should exit.
    // false (0) otherwise, meaning the thread should keep running.
    while (!kthread_should_stop()) {
        printk(KERN_INFO "kthread running: counter = %d\n", counter++);
        ssleep(1);
    }

    printk(KERN_INFO "kthread stopping\n");
    return 0;
}

static int __init kthread_demo_init(void)
{
    printk(KERN_INFO "kthread demo init\n");

    // Creates a new kernel thread but does not start it immediately.
    my_thread = kthread_create(thread_fn, NULL, "my_kthread");
    if (IS_ERR(my_thread))
        return PTR_ERR(my_thread);

    // Wakes up the thread so it starts executing thread_fn.
    wake_up_process(my_thread);

    return 0;
}

static void __exit kthread_demo_exit(void)
{
    if (my_thread)
        // Signals the thread to stop and waits until it exits.
        kthread_stop(my_thread);

    printk(KERN_INFO "kthread demo exited\n");
}

module_init(kthread_demo_init);
module_exit(kthread_demo_exit);
