#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    unsigned long address;
    FILE *fp;

    fp = fopen("/proc/kernel_memory_address", "r");

    if (!fp) {
        perror("fopen");
        return 1;
    }

    if (fscanf(fp, "%lx", &address) != 1) {
        fprintf(stderr, "Could not read kernel address\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    printf("Kernel address: 0x%lx\n", address);
    printf("Trying to read kernel memory...\n");

    unsigned long value = *(unsigned long *)address;

    printf("Value = 0x%lx\n", value);

    return 0;
}
