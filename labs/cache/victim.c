#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include "shared.h"

int main(void) {
    // 1. Create POSIX shared memory region
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Set memory size
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // 3. Map shared memory into victim's address space
    uint8_t *shared_array = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_array == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Secret key exists ONLY in victim's memory space
    uint8_t secret_key = 42;
    printf("[Victim] Process running (PID: %d). Shared array mapped at %p\n", getpid(), (void*)shared_array);
    printf("[Victim] Continuously accessing index %d...\n", secret_key);

    while (1) {
        // Access target array line based on secret key
        volatile uint8_t dummy = shared_array[secret_key * STRIDE];
        (void)dummy;

        // Pause briefly to give attacker window to flush and reload
        usleep(1000);
    }

    return 0;
}
