#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>        /* For struct file_operations */
#include <linux/miscdevice.h>/* For struct miscdevice, misc_register(), misc_deregister() */
#include <linux/uaccess.h>   /* For copy_from_user(), copy_to_user() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demonstration of Misc Device Registration and IOCTL Communication");

#define DEMO_MISC_NAME "demo_dev"

/*
 * Define IOCTL commands using standard kernel macros:
 * _IOW(type, nr, datatype)  - User passes data TO kernel
 * _IOR(type, nr, datatype)  - User receives data FROM kernel
 */
#define DEMO_IOC_MAGIC 'k'
#define DEMO_IOC_SET_VAL _IOW(DEMO_IOC_MAGIC, 1, int)
#define DEMO_IOC_GET_VAL _IOR(DEMO_IOC_MAGIC, 2, int)

static int stored_value = 42;

/*
 * Unlocked IOCTL handler (Runs in PROCESS CONTEXT when a user app calls ioctl()).
 * Safe to sleep, allocate memory, or access user-space memory.
 */
static long demo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int val;

    switch (cmd) {
    case DEMO_IOC_SET_VAL:
        /* Copy data sent from user space into kernel space */
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;

        stored_value = val;
        pr_info("misc_ioctl_demo: Updated stored_value to %d via IOCTL\n", stored_value);
        break;

    case DEMO_IOC_GET_VAL:
        /* Copy data from kernel space back to user space */
        if (copy_to_user((int __user *)arg, &stored_value, sizeof(int)))
            return -EFAULT;

        pr_info("misc_ioctl_demo: Read stored_value %d via IOCTL\n", stored_value);
        break;

    default:
        pr_err("misc_ioctl_demo: Invalid IOCTL command 0x%x\n", cmd);
        return -ENOTTY; /* Standard error for invalid IOCTL */
    }

    return 0;
}

/* File operations structure binding system calls to driver functions */
static const struct file_operations demo_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = demo_ioctl,
};

/* Misc device configuration structure */
static struct miscdevice demo_dev = {
    .minor = MISC_DYNAMIC_MINOR, /* Kernel automatically assigns a free minor number */
    .name  = DEMO_MISC_NAME,     /* Device node created under /dev/demo_dev */
    .fops  = &demo_fops,
};

static int __init demo_init(void)
{
    int ret;

    /* Register device under /dev/demo_dev */
    ret = misc_register(&demo_dev);
    if (ret) {
        pr_err("misc_ioctl_demo: Failed to register device /dev/%s (err=%d)\n",
               DEMO_MISC_NAME, ret);
        return ret;
    }

    pr_info("misc_ioctl_demo: Device registered at /dev/%s\n", DEMO_MISC_NAME);
    return 0;
}

static void __exit demo_exit(void)
{
    /* Always unregister device node on rmmod */
    misc_deregister(&demo_dev);
    pr_info("misc_ioctl_demo: Device /dev/%s unregistered\n", DEMO_MISC_NAME);
}

module_init(demo_init);
module_exit(demo_exit);
