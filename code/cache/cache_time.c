#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[10 * 4096];

int main(void)
{
    unsigned int junk = 0;

    /*
     * 'register' KEYWORD:
     * Asks the compiler to store 'time1' and 'time2' inside CPU registers 
     * (e.g., %rax, %rcx, %r8) rather than allocating space on the stack (in RAM).
     *
     * WHY IT MATTERS HERE:
     * If 'time1' and 'time2' were stored on the stack, reading or writing to them
     * would generate stack memory accesses during measurement. These extra stack 
     * reads/writes would introduce unwanted L1 cache hits/misses and corrupt 
     * high-precision RDTSCP timing.
     */
    register uint64_t time1, time2;

    /*
     * 'volatile' KEYWORD:
     * Tells the compiler that the memory pointed to by 'addr' can change or be 
     * accessed in ways the compiler cannot predict, so it MUST NOT optimize away 
     * or reorder any reads/writes through this pointer.
     *
     * WHY IT MATTERS HERE:
     * Down in the loop, we execute: junk = *addr;
     * Without 'volatile', an optimizing compiler (e.g., gcc -O2) sees that 'junk' 
     * isn't doing anything useful, so it would delete 'junk = *addr;' as "dead code". 
     * If that happens, no memory access occurs, and you end up measuring 0 CPU cycles!
     * 'volatile' forces the compiler to generate an actual hardware 'mov' instruction.
     */
    volatile uint8_t *addr;

    int i;

    // Initialize the array.
    for (i = 0; i < 10; i++)
        array[i * 4096] = 1;

    // Flush the array from the CPU cache.
    for (i = 0; i < 10; i++)
        _mm_clflush(&array[i * 4096]);

    // Access some of the array items.
    // These accesses bring entries 3 and 7 back into the cache.
    array[3 * 4096] = 100;
    array[7 * 4096] = 200;

    // Measure the access time for each entry.
    for (i = 0; i < 10; i++) {
        addr = &array[i * 4096];

        time1 = __rdtscp(&junk);
        junk = *addr;             // Forced memory read due to 'volatile'
        time2 = __rdtscp(&junk) - time1;

        printf("Access time for array[%d*4096]: %d CPU cycles\n",
               i, (int)time2);
    }

    return 0;
}
