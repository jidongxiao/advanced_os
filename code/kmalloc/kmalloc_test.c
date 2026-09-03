// kmalloc_test.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h> // for kmalloc and kfree

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("Kernel module example: kmalloc/kfree");
MODULE_VERSION("1.0");

static char *buffer;

static int __init kmalloc_init(void)
{
    printk(KERN_INFO "kmalloc_test: init: this is a test!\n");

    // Allocate 128 bytes of kernel memory, here we need to use the flag GFP_KERNEL
    /*
     * GFP_KERNEL: GFP stands for "Get Free Pages."
     *
     * GFP_KERNEL tells the kernel that this memory allocation is being
     * performed in normal kernel/process context, where the kernel is
     * allowed to sleep while waiting for memory to become available.
     *
     * An interrupt handler cannot sleep because it runs in atomic context.
     * An interrupt handler may also be holding a lock. If it were allowed
     * to sleep while holding that lock, another process or CPU might wait
     * indefinitely for the lock, potentially causing a deadlock.
     *
     * For allocations where sleeping is not allowed, such as in an
     * interrupt handler, a different GFP flag such as GFP_ATOMIC is used.
     */
    buffer = kmalloc(128, GFP_KERNEL);
    if (!buffer) {
        printk(KERN_ALERT "kmalloc_test: Failed to allocate memory\n");
        return -ENOMEM;
    }

    snprintf(buffer, 128, "This is a test string stored in kmalloc buffer");
    printk(KERN_INFO "kmalloc_test: buffer content: %s\n", buffer);

    return 0;
}

static void __exit kmalloc_exit(void)
{
    printk(KERN_INFO "kmalloc_test: exit: test finished!\n");
    // Free allocated memory
    if (buffer) {
        kfree(buffer);
        printk(KERN_INFO "kmalloc_test: freed buffer memory\n");
    }
}

module_init(kmalloc_init);
module_exit(kmalloc_exit);
