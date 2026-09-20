#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>

// STRIDE set to 4096 bytes (1 standard memory page) to ensure each index maps to a 
// distinct page frame and cache line, mitigating hardware spatial prefetching across boundaries.
#define STRIDE 4096
#define NUM_ELEMENTS 256
#define ARRAY_SIZE (NUM_ELEMENTS * STRIDE)

// Probe array aligned to 4096-byte boundary to align memory lines with page frames.
uint8_t probe_array[ARRAY_SIZE] __attribute__((aligned(4096)));

/**
 * Measures the read access latency of a memory address in CPU clock cycles.
 * 
 * Uses hardware memory fences (_mm_mfence, _mm_lfence) and __rdtscp to enforce 
 * strict instruction serialization and prevent out-of-order execution around the memory read.
 */
static inline uint64_t measure_access_latency(volatile uint8_t *addr) {
    uint64_t cycles_start, cycles_end;
    unsigned int dummy;

    // Enforce memory load/store serialization before starting the timer
    _mm_mfence();
    _mm_lfence();
    cycles_start = __rdtscp(&dummy); // Read Time-Stamp Counter and CPU ID

    // Read target memory location
    (void)*addr;

    // Ensure memory load completes before reading end time
    _mm_lfence();
    cycles_end = __rdtscp(&dummy);
    _mm_mfence();

    return cycles_end - cycles_start;
}

/**
 * Flushes a specific cache line from all cache levels (L1, L2, L3).
 */
static inline void flush_line(volatile uint8_t *addr) {
    _mm_clflush((void*)addr); // Execute CLFLUSH instruction
    _mm_mfence();             // Guarantee eviction completion across store queues
    _mm_lfence();
}

/**
 * Calibrates the CPU hit vs. miss cycle threshold dynamically.
 */
uint64_t calibrate_threshold(void) {
    // Avoid index 0 due to potential page-table or runtime metadata pollution
    volatile uint8_t *dummy_ptr = &probe_array[1 * STRIDE]; 
    uint64_t hit_cycles = 0;
    uint64_t miss_cycles = 0;
    const int rounds = 500;

    // Measure average L1 Cache Hit latency
    (void)*dummy_ptr; // Ensure line is loaded into cache
    for (int i = 0; i < rounds; i++) {
        hit_cycles += measure_access_latency(dummy_ptr);
    }
    hit_cycles /= rounds;

    // Measure average DRAM / Main Memory Miss latency
    for (int i = 0; i < rounds; i++) {
        flush_line(dummy_ptr);
        miss_cycles += measure_access_latency(dummy_ptr);
    }
    miss_cycles /= rounds;

    // Set decision boundary between hit latency and miss latency
    uint64_t threshold = hit_cycles + ((miss_cycles - hit_cycles) / 3);
    printf("[*] Calibration complete: L1 Hit ~%lu cycles | Miss ~%lu cycles\n", hit_cycles, miss_cycles);
    printf("[*] Calculated Threshold: %lu cycles\n\n", threshold);

    return threshold;
}

/**
 * Simulates victim invocation within the same address space.
 */
void victim_access(uint8_t secret_byte) {
    volatile uint8_t *target = &probe_array[secret_byte * STRIDE];
    (void)*target;
}

int main(void) {
    // Initialize array pages to force physical page allocation (avoiding copy-on-write zero pages)
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        probe_array[i * STRIDE] = 1;
    }

    uint64_t threshold = calibrate_threshold();
    uint8_t secret_byte = 42;

    // Populate index permutation array
    uint32_t probe_order[NUM_ELEMENTS];
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        probe_order[i] = i;
    }

    // Fisher-Yates shuffle to randomize measurement order and defeat hardware stream prefetchers
    srand(1337);
    for (int i = NUM_ELEMENTS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        uint32_t temp = probe_order[i];
        probe_order[i] = probe_order[j];
        probe_order[j] = temp;
    }

    // --- STEP 1: FLUSH ALL LINES ---
    // Perform multiple passes to clear all target lines from L1d/L2/LLC caches
    for (int pass = 0; pass < 3; pass++) {
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            flush_line(&probe_array[i * STRIDE]);
        }
    }

    // Brief delay to allow pipeline and bus drain
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

        // Skip index 0 to avoid false positives from runtime memory metadata
        if (mix_index == 0) continue;

        volatile uint8_t *addr = &probe_array[mix_index * STRIDE];

        // Compiler barrier: prevents GCC from reordering index calculations across measure_access_latency
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
