#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "ioctl_example"
#define CLASS_NAME "ioctl_class"

#define IOCTL_HELLO _IO(240, 0)
#define IOCTL_SET_VALUE _IOW(240, 1, int)
#define IOCTL_GET_VALUE _IOR(240, 2, int)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("IOCTL example");

static int stored_value = 0;
static int major_number;
static struct class*  ioctl_class  = NULL;
static struct device* ioctl_device = NULL;

// ioctl handler
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;

    switch(cmd) {
        case IOCTL_HELLO:
            printk(KERN_INFO "ioctl_example: Hello from kernel!\n");
            break;

        case IOCTL_SET_VALUE:
            if (copy_from_user(&value, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            stored_value = value;
            printk(KERN_INFO "ioctl_example: Stored value set to %d\n", stored_value);
            break;

        case IOCTL_GET_VALUE:
            value = stored_value;
            if (copy_to_user((int __user *)arg, &value, sizeof(int)))
                return -EFAULT;
            printk(KERN_INFO "ioctl_example: Returned value %d\n", value);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

// file operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = device_ioctl,
};

static int __init ioctl_init(void)
{
    // register character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "ioctl_example: failed to register device\n");
        return major_number;
    }
    printk(KERN_INFO "ioctl_example: registered with major %d\n", major_number);

    // create device class
    ioctl_class = class_create(CLASS_NAME);
    if (IS_ERR(ioctl_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(ioctl_class);
    }

    // create device node in /dev
    ioctl_device = device_create(ioctl_class, NULL, MKDEV(major_number, 0),
                                 NULL, DEVICE_NAME);
    if (IS_ERR(ioctl_device)) {
        class_destroy(ioctl_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(ioctl_device);
    }

    printk(KERN_INFO "ioctl_example: device created successfully\n");
    return 0;
}

static void __exit ioctl_exit(void)
{
    device_destroy(ioctl_class, MKDEV(major_number, 0));
    class_destroy(ioctl_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "ioctl_example: module unloaded\n");
}

module_init(ioctl_init);
module_exit(ioctl_exit);

