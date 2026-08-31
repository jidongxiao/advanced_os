#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

static int __init jiffies_demo_init(void)
{
    unsigned long start = jiffies;
    printk(KERN_INFO "Module loaded at jiffies=%lu\n", start);
    // simulate work
    mdelay(100);
    printk(KERN_INFO "After 100ms, jiffies=%lu\n", jiffies);
    return 0;
}

static void __exit jiffies_demo_exit(void)
{
    printk(KERN_INFO "Exiting jiffies demo module.\n");
}

module_init(jiffies_demo_init);
module_exit(jiffies_demo_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example module for using the jiffies variable");
