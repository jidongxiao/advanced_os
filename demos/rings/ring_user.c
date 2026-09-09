#include <stdio.h>

int main(void)
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

    printf("User program:\n");
    printf("  CS  = 0x%04x\n", cs);
    printf("  CPL = %u\n", cpl);

    if (cpl == 3)
        printf("  Running at Ring 3 (user mode)\n");
    else
        printf("  Unexpected privilege level\n");

    return 0;
}
