#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "my_proc"
#define INPUT_SIZE 512

static ssize_t my_proc_write(struct file *file,
                             const char __user *buffer,
                             size_t count,
                             loff_t *pos)
{
    char input[INPUT_SIZE];

    int pid_number;
    unsigned long user_address;
    char filename[256];

    /*
     * Do not allow the input to exceed our buffer.
     */
    if (count >= sizeof(input))
        return -EINVAL;

    /*
     * Copy the string from user space into the kernel buffer.
     */
    if (copy_from_user(input, buffer, count))
        return -EFAULT;

    input[count] = '\0';

    /*
     * Parse:
     *
     *     PID ADDRESS FILE
     *
     * Example:
     *
     *     12345 0x7f1234000000 data.bin
     */
    if (sscanf(input, "%d %lx %255s",
               &pid_number,
               &user_address,
               filename) != 3) {

        pr_err("my_proc: invalid input\n");
        pr_info("Usage: echo \"PID ADDRESS FILE\" > /proc/%s\n",
                PROC_NAME);

        return -EINVAL;
    }

    /*
     * Just print the values for demonstration.
     */
    pr_info("my_proc: PID     = %d\n", pid_number);
    pr_info("my_proc: Address = 0x%lx\n", user_address);
    pr_info("my_proc: File    = %s\n", filename);

    return count;
}

static const struct proc_ops my_proc_ops = {
    .proc_write = my_proc_write,
};

static int __init my_proc_init(void)
{
    if (proc_create(PROC_NAME, 0666, NULL, &my_proc_ops) == NULL) {
        pr_err("my_proc: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("my_proc: loaded\n");

    return 0;
}

static void __exit my_proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);

    pr_info("my_proc: unloaded\n");
}

module_init(my_proc_init);
module_exit(my_proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Simple /proc input parsing example");
