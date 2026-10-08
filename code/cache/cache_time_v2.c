#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

uint8_t array[10 * 4096];

int main(void)
{
    unsigned int junk = 0;
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    int i;

    // 1. Initialize memory pages so page tables are fully populated
    for (i = 0; i < 10; i++)
        array[i * 4096] = 1;

    // 2. Flush entire array from cache
    for (i = 0; i < 10; i++)
        _mm_clflush(&array[i * 4096]);

    _mm_mfence();

    // 3. Bring ONLY 3 and 7 into cache
    array[3 * 4096] = 100;
    array[7 * 4096] = 200;

    _mm_mfence();

    // 4. Probe loop: use non-sequential index layout
    int indices[10] = {7, 1, 4, 0, 9, 3, 6, 2, 8, 5};

    for (int k = 0; k < 10; k++) {
        i = indices[k];
        addr = &array[i * 4096];

        // Read timestamp
        time1 = __rdtscp(&junk);
        junk = *addr; // Memory load
        time2 = __rdtscp(&junk) - time1;

        printf("Access time for array[%d*4096]: %d CPU cycles\n", i, (int)time2);
    }

    return 0;
}

