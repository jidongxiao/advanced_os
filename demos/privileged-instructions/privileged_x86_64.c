#include <linux/kernel.h>
#include <linux/module.h>

static int __init test_init(void)
{
    unsigned long cr3;

    asm volatile(
        "mov %%cr3, %0"
        : "=r"(cr3)
    );

    pr_info("Kernel: CR3 = %lx\n", cr3);

    return 0;
}

static void __exit test_exit(void)
{
    pr_info("Privileged instruction demo module unloaded.\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demonstrate privileged CR3 access from kernel space");
