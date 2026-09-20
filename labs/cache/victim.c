#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "shared.h"

int main(void) {
    // 1. Create or open a POSIX shared memory object.
    // O_CREAT: Create the region if it doesn't exist.
    // O_RDWR: Read/Write access permissions.
    // 0666: Read and write access permissions for owner, group, and others.
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Set the size of the shared memory region to accommodate all probe elements.
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // 3. Map the shared memory object into the process virtual address space.
    // MAP_SHARED ensures modifications to this region are visible to other processes 
    // mapping the same shared memory file descriptor (e.g., the attacker process).
    uint8_t *shared_array = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_array == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Define the secret offset value to be accessed
    uint8_t secret_key = 42;
    printf("[Victim] Running. Shared array mapped at %p\n", (void*)shared_array);

    // Continuous loop simulating a service executing secret-dependent array accesses
    while (1) {
        // Access the specific offset corresponding to secret_key * STRIDE.
        // The volatile keyword forces the compiler to generate an explicit read instruction 
        // rather than optimizing out the unused variable dereference.
        volatile uint8_t dummy = shared_array[secret_key * STRIDE];
        (void)dummy;

        // Sleep briefly to prevent pinning a CPU core at 100% utilization
        usleep(1000);
    }

    return 0;
}
