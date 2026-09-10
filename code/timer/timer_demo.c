#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static struct timer_list periodic_timer;

static void timer_callback(struct timer_list *t)
{
    printk(KERN_INFO "Timer fired!\n");

    mod_timer(&periodic_timer,
              jiffies + msecs_to_jiffies(200));
}

static int __init timer_demo_init(void)
{
    /*
     * timer_setup() initializes the timer object and associates
     * timer_callback() with it.
     */
    timer_setup(&periodic_timer, timer_callback, 0);

    /*
     * Schedule the timer to fire 200 ms from now.
     */
    mod_timer(&periodic_timer,
              jiffies + msecs_to_jiffies(200));

    return 0;
}

static void __exit timer_demo_exit(void)
{
    /*
     * Cancel the timer and wait for a currently executing callback
     * to finish before unloading the module.
     */
    timer_delete_sync(&periodic_timer);
}

module_init(timer_demo_init);
module_exit(timer_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example Linux kernel timer");
