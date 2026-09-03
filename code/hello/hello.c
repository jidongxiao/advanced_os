#include <linux/module.h> // for module functions
#include <linux/kernel.h> // for printk

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("Minimal Hello World Module");
MODULE_VERSION("1.0");

static int __init hello_init(void)
{
    printk(KERN_INFO "hello: init: Hello, world!\n");
    return 0; // success
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "hello: exit: Goodbye, world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
