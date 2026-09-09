#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init ring_init(void)
{
    unsigned short cs;
    unsigned int cpl;
    /*
     * CS = Code Segment register.
     *
     * The name comes from the original x86 architecture, where CS
     * identified the segment containing the currently executing code.
     *
     * In modern x86-64, segmentation is largely disabled for normal
     * 64-bit code. CS is still used by the CPU to identify the current
     * code-segment context and its privilege level.
     *
     * The low two bits of CS indicate the privilege level of the
     * currently executing code:
     *
     *     0 -> Ring 0 (kernel)
     *     3 -> Ring 3 (user)
     *
     * Therefore, we can read CS and extract its lowest two bits to
     * determine the Current Privilege Level (CPL).
     */
    asm volatile(
        "mov %%cs, %0"
        : "=r"(cs)
    );

    cpl = cs & 0x3;

    pr_info("Kernel module:\n");
    pr_info("  CS  = 0x%04x\n", cs);
    pr_info("  CPL = %u\n", cpl);

    if (cpl == 0)
        pr_info("  Running at Ring 0 (kernel mode)\n");

    return 0;
}

static void __exit ring_exit(void)
{
    pr_info("Ring privilege demo module unloaded.\n");
}

module_init(ring_init);
module_exit(ring_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demonstrate x86-64 CPL and Ring 0/Ring 3");
