#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "shared.h"

int main() {
    // 1. Create POSIX shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Set size of shared memory region
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // 3. Map memory into victim process address space
    uint8_t *shared_array = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_array == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    uint8_t secret_key = 42;
    printf("[Victim] Running. Shared array mapped at %p\n", (void*)shared_array);

    while (1) {
        volatile uint8_t dummy = shared_array[secret_key * STRIDE];
        (void)dummy;
        usleep(1000);
    }

    return 0;
}
