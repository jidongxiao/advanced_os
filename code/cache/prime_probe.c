#include <stdio.h>
#include <stdint.h>
#include <emmintrin.h>
#include <x86intrin.h>

// Allocate a buffer large enough to span multiple 4KB pages.
// Using static allocation ensures page tables are mapped once initialized.
#define PAGE_SIZE 4096
#define NUM_PAGES 16

uint8_t buffer[NUM_PAGES * PAGE_SIZE];

/*
 * Helper: Measure read latency to a specific memory address in CPU cycles.
 */
static inline uint64_t measure_access_time(volatile uint8_t *addr) {
    unsigned int junk = 0;
    register uint64_t time1, time2;

    _mm_mfence();
    _mm_lfence();
    time1 = __rdtsc();
    _mm_lfence();

    (void)*addr; // Forced read

    _mm_lfence();
    time2 = __rdtscp(&junk) - time1;

    return time2;
}

int main(void) {
    // 1. Warm up memory pages to populate Virtual Memory page tables
    for (int i = 0; i < NUM_PAGES; i++) {
        buffer[i * PAGE_SIZE] = 1;
    }

    // Target address (represents Victim's sensitive data line)
    // Page offset = 0x000 -> Maps to L1 Cache Set #0
    volatile uint8_t *victim_addr = &buffer[0 * PAGE_SIZE];

    // Eviction set (represents Attacker's priming addresses)
    // All these addresses share the same page offset (0x000) -> Map to Cache Set #0
    volatile uint8_t *prime_addrs[8];
    for (int i = 1; i <= 8; i++) {
        prime_addrs[i - 1] = &buffer[i * PAGE_SIZE];
    }

    printf("=== DEMONSTRATING PRIME + PROBE ===\n\n");

    // =========================================================================
    // SCENARIO A: Victim DOES NOT access the cache set
    // =========================================================================
    printf("Scenario A: Victim is INACTIVE\n");

    // STEP 1: PRIME
    // Attacker fills Cache Set #0 by reading all addresses in the eviction set
    for (int i = 0; i < 8; i++) {
        (void)*prime_addrs[i];
    }

    // STEP 2: VICTIM PHASE (Victim does nothing)

    // STEP 3: PROBE
    // Attacker re-reads its own addresses and measures execution time
    uint64_t total_time_inactive = 0;
    for (int i = 0; i < 8; i++) {
        total_time_inactive += measure_access_time(prime_addrs[i]);
    }
    printf("  Probe Total Time (Victim inactive) : %lu cycles (FAST -> All Hits)\n\n",
           total_time_inactive);

    // =========================================================================
    // SCENARIO B: Victim ACCESSES the cache set
    // =========================================================================
    printf("Scenario B: Victim is ACTIVE\n");

    // STEP 1: PRIME
    // Attacker fills Cache Set #0 again
    for (int i = 0; i < 8; i++) {
        (void)*prime_addrs[i];
    }

    // STEP 2: VICTIM PHASE
    // Victim accesses its target address mapping to Set #0, kicking out an attacker line
    (void)*victim_addr;

    // STEP 3: PROBE
    // Attacker re-reads its eviction set
    uint64_t total_time_active = 0;
    for (int i = 0; i < 8; i++) {
        total_time_active += measure_access_time(prime_addrs[i]);
    }
    printf("  Probe Total Time (Victim active)   : %lu cycles (SLOW -> Cache Miss!)\n\n",
           total_time_active);

    return 0;
}
