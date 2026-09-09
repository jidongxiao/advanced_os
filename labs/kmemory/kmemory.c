#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "kernel_memory_address"

static unsigned long *kernel_memory;
static struct proc_dir_entry *proc_entry;

static ssize_t proc_read(struct file *file,
                         char __user *buffer,
                         size_t count,
                         loff_t *pos)
{
    char output[64];
    int len;

    if (*pos > 0)
        return 0;

    len = snprintf(output, sizeof(output),
                   "%px\n", kernel_memory);

    if (copy_to_user(buffer, output, len))
        return -EFAULT;

    *pos = len;
    return len;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

static int __init kmemory_init(void)
{
    kernel_memory = kmalloc(sizeof(unsigned long), GFP_KERNEL);

    if (!kernel_memory)
        return -ENOMEM;

    *kernel_memory = 0x123456789ABCDEF0UL;

    proc_entry = proc_create(PROC_NAME, 0444, NULL, &proc_fops);

    if (!proc_entry) {
        kfree(kernel_memory);
        return -ENOMEM;
    }

    pr_info("Allocated kernel memory at %px\n", kernel_memory);
    pr_info("Value = 0x%lx\n", *kernel_memory);

    return 0;
}

static void __exit kmemory_exit(void)
{
    proc_remove(proc_entry);
    kfree(kernel_memory);

    pr_info("Kernel memory released\n");
}

module_init(kmemory_init);
module_exit(kmemory_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel memory access demonstration");
