#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>

static int __init get_page_init(void)
{
    pid_t pid = 12345;
    unsigned long user_address = 0x7f1234000000;

    struct pid *pid_struct;
    struct task_struct *task;
    struct mm_struct *mm;
    struct page *page;

    unsigned long pfn;
    long ret;

    /*
     * Find the process from its PID.
     */
    pid_struct = find_get_pid(pid);

    if (!pid_struct)
        return -ESRCH;

    task = get_pid_task(pid_struct, PIDTYPE_PID);

    put_pid(pid_struct);

    if (!task)
        return -ESRCH;

    /*
     * Get the process's memory descriptor.
     */
    mm = get_task_mm(task);

    if (!mm) {
        put_task_struct(task);
        return -EINVAL;
    }

    /*
     * Translate the user virtual address into
     * the struct page describing the physical page.
     */
    mmap_read_lock(mm);

    ret = get_user_pages_remote(mm,
                                user_address,
                                1,
                                FOLL_GET,
                                &page,
                                NULL);

    mmap_read_unlock(mm);

    if (ret != 1) {
        pr_err("get_user_pages_remote() failed: %ld\n", ret);

        mmput(mm);
        put_task_struct(task);

        return -EFAULT;
    }

    /*
     * Convert struct page into a physical page frame number.
     */
    pfn = page_to_pfn(page);

    pr_info("PID: %d\n", pid);
    pr_info("Virtual address: 0x%lx\n", user_address);
    pr_info("PFN: %lu\n", pfn);
    pr_info("Physical address: 0x%lx\n", pfn << PAGE_SHIFT);

    /*
     * Release the reference obtained by FOLL_GET.
     */
    put_page(page);

    mmput(mm);
    put_task_struct(task);

    return 0;
}

static void __exit get_page_exit(void)
{
    pr_info("get_page: unloaded\n");
}

module_init(get_page_init);
module_exit(get_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Example of translating a user virtual address to a physical page");
