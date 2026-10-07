#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/highmem.h>

#define PROC_NAME "pagecache_test"
#define INPUT_SIZE 512

static struct proc_dir_entry *proc_entry;

/*
 * Compare the contents of two physical pages.
 *
 * Both pages are temporarily mapped into kernel virtual memory.
 */
static int compare_pages(struct page *page1, struct page *page2)
{
    void *addr1;
    void *addr2;
    int result;

    addr1 = kmap_local_page(page1);
    addr2 = kmap_local_page(page2);

    result = memcmp(addr1, addr2, PAGE_SIZE);

    kunmap_local(addr2);
    kunmap_local(addr1);

    return result;
}


static ssize_t pagecache_test_write(struct file *file,
                                    const char __user *buffer,
                                    size_t count,
                                    loff_t *pos)
{
    char input[INPUT_SIZE];
    pid_t pid_number;
    unsigned long user_address;
    char filename[256];

    struct pid *pid_struct;
    struct task_struct *task;
    struct mm_struct *mm;

    struct page *user_page = NULL;
    struct page *cache_page = NULL;

    struct file *test_file;
    struct address_space *mapping;

    long ret;
    unsigned long pfn_user;
    unsigned long pfn_cache;

    if (count >= INPUT_SIZE)
        return -EINVAL;

    if (copy_from_user(input, buffer, count))
        return -EFAULT;

    input[count] = '\0';

    /*
     * Expected input:
     *
     * PID USER_VIRTUAL_ADDRESS FILE_NAME
     *
     * Example:
     *
     * 12345 0x7f1234000000 /home/xiaoj8/test/data.bin
     */
    if (sscanf(input, "%d %lx %255s",
               &pid_number,
               &user_address,
               filename) != 3) {
        pr_err("pagecache_test: invalid input\n");
        pr_info("Usage: echo \"PID ADDRESS FILE\" > /proc/%s\n",
                PROC_NAME);
        return -EINVAL;
    }

    pr_info("\n");
    pr_info("========================================\n");
    pr_info("Page Cache vs User Buffer Experiment\n");
    pr_info("========================================\n");

    pr_info("PID:              %d\n", pid_number);
    pr_info("User virtual addr: 0x%lx\n", user_address);
    pr_info("File:             %s\n", filename);


    /*
     * ------------------------------------------------------------
     * Find the process.
     * ------------------------------------------------------------
     */

    pid_struct = find_get_pid(pid_number);

    if (pid_struct == NULL) {
        pr_err("pagecache_test: PID not found\n");
        return -ESRCH;
    }

    task = get_pid_task(pid_struct, PIDTYPE_PID);

    put_pid(pid_struct);

    if (task == NULL) {
        pr_err("pagecache_test: task not found\n");
        return -ESRCH;
    }

    mm = get_task_mm(task);

    if (mm == NULL) {
        pr_err("pagecache_test: process has no mm_struct\n");
        put_task_struct(task);
        return -EINVAL;
    }


    /*
     * ------------------------------------------------------------
     * Find the physical page containing the USER BUFFER.
     * ------------------------------------------------------------
     *
     * get_user_pages_remote() walks the process's page tables.
     */
    mmap_read_lock(mm);
    ret = get_user_pages_remote(mm,
                                user_address,
                                1,
                                FOLL_GET,
                                &user_page,
                                NULL);
    mmap_read_unlock(mm);

    if (ret != 1) {
        pr_err("pagecache_test: get_user_pages_remote failed: %ld\n",
               ret);

        mmput(mm);
        put_task_struct(task);

        return -EFAULT;
    }

    pfn_user = page_to_pfn(user_page);


    /*
     * ------------------------------------------------------------
     * Open the file and find its page-cache page.
     * ------------------------------------------------------------
     */

    test_file = filp_open(filename, O_RDONLY | O_LARGEFILE, 0);

    if (IS_ERR(test_file)) {
        pr_err("pagecache_test: filp_open failed\n");

        put_page(user_page);
        mmput(mm);
        put_task_struct(task);

        return PTR_ERR(test_file);
    }

    mapping = file_inode(test_file)->i_mapping;

    /*
     * Page index 0 corresponds to file offset 0.
     */
    cache_page = find_get_page(mapping, 0);

    if (cache_page == NULL) {
        pr_err("pagecache_test: page is NOT in page cache\n");

        filp_close(test_file, NULL);
        put_page(user_page);
        mmput(mm);
        put_task_struct(task);

        return -ENOENT;
    }

    pfn_cache = page_to_pfn(cache_page);


    /*
     * ------------------------------------------------------------
     * Print the physical pages.
     * ------------------------------------------------------------
     */

    pr_info("\n");
    pr_info("USER BUFFER:\n");
    pr_info("  virtual address = 0x%lx\n", user_address);
    pr_info("  PFN              = %lu\n", pfn_user);
    pr_info("  physical address = 0x%llx\n",
            (unsigned long long)pfn_user << PAGE_SHIFT);

    pr_info("\n");

    pr_info("PAGE CACHE:\n");
    pr_info("  file offset      = 0\n");
    pr_info("  PFN              = %lu\n", pfn_cache);
    pr_info("  physical address = 0x%llx\n",
            (unsigned long long)pfn_cache << PAGE_SHIFT);


    /*
     * ------------------------------------------------------------
     * Compare the actual contents of the two physical pages.
     * ------------------------------------------------------------
     */

    ret = compare_pages(cache_page, user_page);

    pr_info("\n");

    if (ret == 0)
        pr_info("CONTENTS:           SAME\n");
    else
        pr_info("CONTENTS:           DIFFERENT (memcmp = %ld)\n",
                ret);

    pr_info("\n");

    if (pfn_cache != pfn_user) {
        pr_info("PFNs:               DIFFERENT\n");
        pr_info("\n");
        pr_info("RESULT:\n");
        pr_info("  TWO DIFFERENT PHYSICAL PAGES\n");
        pr_info("  CONTAIN THE SAME FILE DATA.\n");
    } else {
        pr_info("PFNs:               SAME\n");
        pr_info("\n");
        pr_info("RESULT:\n");
        pr_info("  The pages are physically shared.\n");
    }

    pr_info("========================================\n");


    /*
     * Release references.
     */

    put_page(cache_page);
    filp_close(test_file, NULL);

    put_page(user_page);

    mmput(mm);
    put_task_struct(task);

    return count;
}


static const struct proc_ops pagecache_test_ops = {
    .proc_write = pagecache_test_write,
};


static int __init pagecache_probe_init(void)
{
    proc_entry = proc_create(PROC_NAME,
                             0222,
                             NULL,
                             &pagecache_test_ops);

    if (proc_entry == NULL) {
        pr_err("pagecache_test: failed to create /proc/%s\n",
               PROC_NAME);
        return -ENOMEM;
    }

    pr_info("pagecache_test: loaded\n");
    pr_info("Use /proc/%s to run the experiment\n", PROC_NAME);

    return 0;
}


static void __exit pagecache_probe_exit(void)
{
    proc_remove(proc_entry);

    pr_info("pagecache_test: unloaded\n");
}


module_init(pagecache_probe_init);
module_exit(pagecache_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RPI");
MODULE_DESCRIPTION("Demonstrate page-cache and read-buffer physical pages");
