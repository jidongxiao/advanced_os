#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

static int __init list_tasks_init(void)
{
    struct task_struct *task;

    for_each_process(task) {
        printk(KERN_INFO "PID=%d, Name=%s\n", task->pid, task->comm);
    }

    return 0;
}

static void __exit list_tasks_exit(void)
{
    printk(KERN_INFO "Exiting process listing module.\n");
}

module_init(list_tasks_init);
module_exit(list_tasks_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example module for listing tasks");
