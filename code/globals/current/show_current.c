#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

static int __init show_current_init(void)
{
    printk(KERN_INFO "Current PID: %d\n", current->pid);
    printk(KERN_INFO "Current process name: %s\n", current->comm);
    return 0;
}

static void __exit show_current_exit(void)
{
    printk(KERN_INFO "Exiting current demo module.\n");
}

module_init(show_current_init);
module_exit(show_current_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example module for using the current variable");
