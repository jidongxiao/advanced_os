#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("Hello Character Device Module with /dev auto-create");
MODULE_VERSION("1.1");

#define DEVICE_NAME "hello_char"

static int major;
static struct class *hello_class = NULL;
static struct cdev hello_cdev;

static ssize_t hello_read(struct file *filep, char __user *buf, size_t len, loff_t *offset)
{
    char message[] = "Hello from kernel!\n";
    size_t msg_len = sizeof(message);

    if (*offset >= msg_len)
        return 0;

    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (copy_to_user(buf, message + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

static ssize_t hello_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset)
{
    char kbuf[128];
    if (len > sizeof(kbuf) - 1)
        len = sizeof(kbuf) - 1;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';
    printk(KERN_INFO "hello_char: Received from user: %s\n", kbuf);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = hello_read,
    .write = hello_write,
};

static int __init hello_init(void)
{
    dev_t dev;

    // Allocate major number dynamically
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "hello_char: Failed to allocate major number\n");
        return -1;
    }
    major = MAJOR(dev);

    // Initialize cdev
    cdev_init(&hello_cdev, &fops);
    hello_cdev.owner = THIS_MODULE;
    if (cdev_add(&hello_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "hello_char: Failed to add cdev\n");
        return -1;
    }

    // Create class and device to auto-create /dev/hello_char
    hello_class = class_create("hello_class");
    if (IS_ERR(hello_class)) {
        cdev_del(&hello_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "hello_char: Failed to create class\n");
        return PTR_ERR(hello_class);
    }

    device_create(hello_class, NULL, dev, NULL, DEVICE_NAME);

    printk(KERN_INFO "hello_char: Module loaded, device /dev/%s, major=%d\n", DEVICE_NAME, major);
    return 0;
}

static void __exit hello_exit(void)
{
    dev_t dev = MKDEV(major, 0);

    // Remove device and class
    device_destroy(hello_class, dev);
    class_destroy(hello_class);

    // Remove cdev and unregister major number
    cdev_del(&hello_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "hello_char: Module unloaded, /dev/%s removed\n", DEVICE_NAME);
}

module_init(hello_init);
module_exit(hello_exit);
