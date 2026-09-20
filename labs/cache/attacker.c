#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>

#define STRIDE 4096          
#define NUM_ELEMENTS 256
#define ARRAY_SIZE (NUM_ELEMENTS * STRIDE)

uint8_t probe_array[ARRAY_SIZE] __attribute__((aligned(4096)));

static inline uint64_t measure_access_latency(volatile uint8_t *addr) {
    uint64_t cycles_start, cycles_end;
    unsigned int dummy;

    _mm_mfence();
    _mm_lfence();
    cycles_start = __rdtscp(&dummy);

    (void)*addr;

    _mm_lfence();
    cycles_end = __rdtscp(&dummy);
    _mm_mfence();

    return cycles_end - cycles_start;
}

static inline void flush_line(volatile uint8_t *addr) {
    _mm_clflush((void*)addr);
    _mm_mfence();
    _mm_lfence();
}

uint64_t calibrate_threshold(void) {
    volatile uint8_t *dummy_ptr = &probe_array[1 * STRIDE]; // Use non-zero index for calibration
    uint64_t hit_cycles = 0;
    uint64_t miss_cycles = 0;
    const int rounds = 500;

    (void)*dummy_ptr;
    for (int i = 0; i < rounds; i++) {
        hit_cycles += measure_access_latency(dummy_ptr);
    }
    hit_cycles /= rounds;

    for (int i = 0; i < rounds; i++) {
        flush_line(dummy_ptr);
        miss_cycles += measure_access_latency(dummy_ptr);
    }
    miss_cycles /= rounds;

    uint64_t threshold = hit_cycles + ((miss_cycles - hit_cycles) / 3);
    printf("[*] Calibration complete: L1 Hit ~%lu cycles | Miss ~%lu cycles\n", hit_cycles, miss_cycles);
    printf("[*] Calculated Threshold: %lu cycles\n\n", threshold);

    return threshold;
}

void victim_access(uint8_t secret_byte) {
    volatile uint8_t *target = &probe_array[secret_byte * STRIDE];
    (void)*target;
}

int main(void) {
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        probe_array[i * STRIDE] = 1;
    }

    uint64_t threshold = calibrate_threshold();
    uint8_t secret_byte = 42;

    uint32_t probe_order[NUM_ELEMENTS];
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        probe_order[i] = i;
    }
    srand(1337);
    for (int i = NUM_ELEMENTS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        uint32_t temp = probe_order[i];
        probe_order[i] = probe_order[j];
        probe_order[j] = temp;
    }

    // --- STEP 1: FLUSH ALL LINES ---
    for (int pass = 0; pass < 3; pass++) {
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            flush_line(&probe_array[i * STRIDE]);
        }
    }

    for (volatile int k = 0; k < 100000; k++);

    // --- STEP 2: VICTIM ACCESS ---
    victim_access(secret_byte);

    _mm_mfence();
    _mm_lfence();

    // --- STEP 3: RELOAD & MEASURE ---
    printf("%-10s %-15s %-10s\n", "Index", "Latency (cycles)", "Status");
    printf("----------------------------------------\n");

    int hit_count = 0;

    for (int i = 0; i < NUM_ELEMENTS; i++) {
        uint32_t mix_index = probe_order[i];

        // Skip index 0 to avoid base address pollution
        if (mix_index == 0) continue;

        volatile uint8_t *addr = &probe_array[mix_index * STRIDE];

        asm volatile("" ::: "memory");

        uint64_t latency = measure_access_latency(addr);

        if (latency < threshold) {
            printf("Index %-3u: %-15lu [CACHE HIT]\n", mix_index, latency);
            hit_count++;
        }
    }

    if (hit_count == 0) {
        printf("No cache hits detected.\n");
    }

    return 0;
}
