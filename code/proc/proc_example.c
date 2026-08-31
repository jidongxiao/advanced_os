#include <linux/module.h> // Module infrastructure: MODULE_*, module_init(), module_exit()
#include <linux/kernel.h> // Kernel utilities and printk() logging macros
#include <linux/proc_fs.h> // /proc filesystem: struct proc_ops, proc_create(), remove_proc_entry()
#include <linux/uaccess.h> // Safe data transfer between user space and kernel space

#define PROC_NAME "myprocfile"
#define BUFFER_SIZE 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("Example /proc module for 7.0 kernel");

static char proc_buffer[BUFFER_SIZE];
static int proc_buffer_size = 0; // static limits their visibility to this source file

// proc file operations
static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
    .proc_write = proc_write,
};

// read handler
static ssize_t proc_read(struct file *file, char __user *user_buf,
                         size_t count, loff_t *ppos)
{
    if (*ppos > 0)  // EOF. ppos is a pointer to the current file position, initially, the file position is 0.
        return 0;

    if (copy_to_user(user_buf, proc_buffer, proc_buffer_size))
        return -EFAULT;

    *ppos = proc_buffer_size;
    return proc_buffer_size;
}

// write handler
static ssize_t proc_write(struct file *file, const char __user *user_buf,
                          size_t count, loff_t *ppos)
{
    if (count > BUFFER_SIZE - 1)
        count = BUFFER_SIZE - 1;

    if (copy_from_user(proc_buffer, user_buf, count))
        return -EFAULT;

    proc_buffer[count] = '\0';
    proc_buffer_size = count;

    printk(KERN_INFO "proc_example: user wrote: %s\n", proc_buffer);

    return count;
}

static struct proc_dir_entry *proc_entry;

// __init is a kernel-specific annotation that tells the kernel:
// This function is only needed during module initialization, so its memory can be discarded after initialization finishes.
static int __init proc_init(void)
{
    // Create /proc/myprocfile with read/write permissions for everyone.
    // 0666 = rw-rw-rw- (read + write for owner, group, and others).
    // The leading 0 means the number is written in octal.
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);

    if (!proc_entry) {
        printk(KERN_ERR "proc_example: failed to create /proc/%s\n",
               PROC_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "proc_example: /proc/%s created\n", PROC_NAME);
    return 0;
}

// __exit marks this function as module cleanup code.
// It is called when the module is unloaded. For code built into
// the kernel, exit code can be discarded because it cannot be unloaded.
static void __exit proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "proc_example: /proc/%s removed\n", PROC_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
