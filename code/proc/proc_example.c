#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "myprocfile"
#define BUFFER_SIZE 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("Example /proc module for 7.0 kernel");

static char proc_buffer[BUFFER_SIZE];
static int proc_buffer_size = 0;

// read handler
static ssize_t proc_read(struct file *file, char __user *user_buf,
                         size_t count, loff_t *ppos)
{
    if (*ppos > 0)  // EOF
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

// proc file operations
static const struct proc_ops proc_fops = {
    .proc_read  = proc_read,
    .proc_write = proc_write,
};

static int __init proc_init(void)
{
    proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    printk(KERN_INFO "proc_example: /proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "proc_example: /proc/%s removed\n", PROC_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
