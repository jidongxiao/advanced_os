#include <stdio.h>

int main(void)
{
    unsigned long value;

    printf("Trying to read CR3 from user space...\n");

    asm volatile(
        "mov %%cr3, %0"
        : "=r"(value)
    );

    printf("CR3 = %lx\n", value);

    return 0;
}
