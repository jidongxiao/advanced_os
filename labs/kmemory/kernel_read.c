#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static unsigned long address;

module_param(address, ulong, 0444);
MODULE_PARM_DESC(address, "Kernel virtual address to read");

static int __init kernel_read_init(void)
{
    unsigned long value;

    if (address == 0) {
        pr_err("Please provide an address\n");
        return -EINVAL;
    }

    value = *(unsigned long *)address;

    pr_info("Reading kernel address: %px\n",
            (void *)address);

    pr_info("Value = 0x%lx\n", value);

    return 0;
}

static void __exit kernel_read_exit(void)
{
    pr_info("Kernel reader unloaded\n");
}

module_init(kernel_read_init);
module_exit(kernel_read_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel memory reader demonstration");
