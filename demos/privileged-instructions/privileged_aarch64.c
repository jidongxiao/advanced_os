#include <linux/kernel.h>
#include <linux/module.h>

static int __init test_init(void)
{
    unsigned long value;

    /*
     * Read the ARM64 system register TTBR0_EL1.
     *
     * MRS = Move Register from System register.
     *
     * TTBR0_EL1 contains the base address of the translation
     * table used for address translation at Exception Level 1.
     *
     * The CPU checks the current privilege level. This instruction
     * requires EL1 privilege, so it can be executed by the kernel
     * but not by a normal user-space program running at EL0.
     */
    asm volatile(
        "mrs %0, ttbr0_el1"
        : "=r"(value)         // tell GCC "put the output in a general-purpose register of your choice".
                              // The CPU instruction MRS cannot directly write into a C variable in memory. 
                              // The ARM64 instruction operates on CPU registers.
    );

    /*
     * Print the value that was read from TTBR0_EL1 to the
     * kernel log. Use "sudo dmesg" to see this message.
     */
    pr_info("Kernel: TTBR0_EL1 = %lx\n", value);

    return 0;
}

static void __exit test_exit(void)
{
    pr_info("Privileged instruction demo module unloaded.\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demonstrate privileged system-register access from kernel space");
