#include <stdio.h>

int main(void)
{
    unsigned long value;

    printf("Trying to read CR3 from user space...\n");

    asm volatile(
        "mrs %0, ttbr0_el1"
        : "=r"(value)
    );

    printf("CR3 = %lx\n", value);

    return 0;
}
