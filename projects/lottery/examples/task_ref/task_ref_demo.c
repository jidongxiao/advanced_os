#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demo of get_task_struct and put_task_struct reference counting");
MODULE_VERSION("1.0");

static int target_pid = 1; /* Default to PID 1 (init/systemd) */
module_param(target_pid, int, 0444);
MODULE_PARM_DESC(target_pid, "Target PID to safely inspect and reference");

static struct task_struct *safe_task = NULL;

static int __init task_ref_demo_init(void)
{
    struct task_struct *p;
    bool found = false;

    pr_info("[task_ref_demo] Loading module for target PID: %d\n", target_pid);

    /* 
     * STEP 1: Find the target task while holding RCU read lock.
     * RCU protects task list traversal, BUT it does NOT prevent the task 
     * struct from being freed once rcu_read_unlock() is called if the task exits!
     */
    rcu_read_lock();
    for_each_process(p) {
        if (p->pid == target_pid) {
            /* 
             * INCREMENT REFERENCE COUNT:
             * get_task_struct() increments p->usage.
             * Even if the process calls do_exit() and terminates, the kernel 
             * WILL NOT free the 'struct task_struct' memory while usage > 0.
             */
            get_task_struct(p);
            safe_task = p;
            found = true;
            break;
        }
    }
    rcu_read_unlock();

    if (!found) {
        pr_err("[task_ref_demo] Process PID %d not found.\n", target_pid);
        return -ESRCH;
    }

    /* 
     * STEP 2: Safe Execution Outside Lock
     * Even if target_pid dies during msleep(), 'safe_task' remains a valid pointer!
     */
    pr_info("[task_ref_demo] Acquired reference to '%s' (PID %d, usage count bumped).\n",
            safe_task->comm, safe_task->pid);

    /* Simulating work outside of lock/RCU critical section */
    msleep(100);

    pr_info("[task_ref_demo] Safely read state: comm=%s, state=0x%x\n",
            safe_task->comm, safe_task->__state);

    return 0;
}

static void __exit task_ref_demo_exit(void)
{
    if (safe_task) {
        pr_info("[task_ref_demo] Releasing reference to PID %d...\n", safe_task->pid);
        
        /* 
         * DECREMENT REFERENCE COUNT:
         * put_task_struct() decrements p->usage.
         * If the task has already exited and this was the last reference, 
         * the kernel slab allocator will now safely free the memory.
         */
        put_task_struct(safe_task);
        safe_task = NULL;
    }

    pr_info("[task_ref_demo] Module unloaded cleanly.\n");
}

module_init(task_ref_demo_init);
module_exit(task_ref_demo_exit);
