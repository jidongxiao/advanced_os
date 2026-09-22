#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Target Kernel Module for Breakpoint Instrumenting");

#define PROC_FILENAME "target_trigger"

void target_module_function(void);

// Prevent compiler from inlining this function so Kprobes can attach to its symbol
noinline void target_module_function(void) {
    volatile int x = 10;
    volatile int y = 20;
    volatile int z = x + y;

    pr_info("[Target] Executing target_module_function (z = %d)...\n", z);
}
EXPORT_SYMBOL(target_module_function);

// Procfs write callback to trigger the function on user demand
static ssize_t proc_trigger_write(struct file *file, const char __user *buffer,
                                 size_t count, loff_t *ppos) {
    pr_info("[Target] Proc trigger invoked!\n");
    
    // Call the function containing our target breakpoint
    target_module_function();

    return count;
}

static const struct proc_ops proc_trigger_ops = {
    .proc_write = proc_trigger_write,
};

static int __init target_init(void) {
    struct proc_dir_entry *entry;

    entry = proc_create(PROC_FILENAME, 0222, NULL, &proc_trigger_ops);
    if (!entry) {
        pr_err("[Target] Failed to create /proc/%s\n", PROC_FILENAME);
        return -ENOMEM;
    }

    pr_info("[Target] Loaded successfully. Trigger via: echo 1 > /proc/%s\n", PROC_FILENAME);
    return 0;
}

static void __exit target_exit(void) {
    remove_proc_entry(PROC_FILENAME, NULL);
    pr_info("[Target] Unloaded.\n");
}

module_init(target_init);
module_exit(target_exit);
