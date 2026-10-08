#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/moduleparam.h>

/* Allow passing target PID and virtual address via insmod parameters */
static int target_pid = -1;
module_param(target_pid, int, 0644);
MODULE_PARM_DESC(target_pid, "Target process PID");

static unsigned long user_address = 0;
module_param(user_address, ulong, 0644);
MODULE_PARM_DESC(user_address, "Virtual address to translate (in hex, e.g. 0x7f1234000000)");

static int __init get_page_init(void)
{
    struct pid *pid_struct;
    struct task_struct *task;
    struct mm_struct *mm;
    struct page *page = NULL;
    unsigned long pfn;
    long ret;

    /* Fallback: if no PID was provided, default to the process inserting the module */
    if (target_pid <= 0) {
        target_pid = task_pid_nr(current);
        pr_info("get_page: No target_pid specified, defaulting to current process PID: %d\n", target_pid);
    }

    if (user_address == 0) {
        pr_err("get_page: Please provide a valid user_address parameter (e.g., user_address=0x...)\n");
        return -EINVAL;
    }

    /* 1. Find process task structure from PID */
    pid_struct = find_get_pid(target_pid);
    if (!pid_struct) {
        pr_err("get_page: Could not find pid_struct for PID %d\n", target_pid);
        return -ESRCH;
    }

    task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);

    if (!task) {
        pr_err("get_page: Could not find task_struct for PID %d\n", target_pid);
        return -ESRCH;
    }

    /* 2. Get process memory descriptor (mm_struct) */
    mm = get_task_mm(task);
    if (!mm) {
        pr_err("get_page: Process PID %d has no active mm_struct (kernel thread?)\n", target_pid);
        put_task_struct(task);
        return -EINVAL;
    }

    /* 3. Walk page tables and retrieve physical page reference */
    mmap_read_lock(mm);
    ret = get_user_pages_remote(mm,
                                user_address,
                                1,
                                FOLL_GET,
                                &page,
                                NULL);
    mmap_read_unlock(mm);

    if (ret != 1) {
        pr_err("get_page: get_user_pages_remote() failed with error code: %ld\n", ret);
        mmput(mm);
        put_task_struct(task);
        return -EFAULT;
    }

    /* 4. Convert struct page to Physical Frame Number (PFN) */
    pfn = page_to_pfn(page);

    pr_info("========================================\n");
    pr_info("get_page Translation Result\n");
    pr_info("========================================\n");
    pr_info("PID:               %d\n", target_pid);
    pr_info("Virtual address:   0x%lx\n", user_address);
    pr_info("PFN:               %lu\n", pfn);
    pr_info("Physical address:  0x%lx\n", pfn << PAGE_SHIFT);
    pr_info("========================================\n");

    /* 5. Clean up references */
    put_page(page);
    mmput(mm);
    put_task_struct(task);

    return 0;
}

static void __exit get_page_exit(void)
{
    pr_info("get_page: module unloaded\n");
}

module_init(get_page_init);
module_exit(get_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced OS");
MODULE_DESCRIPTION("Translate user virtual address to physical address");
