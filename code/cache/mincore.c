#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

int main(void) {
    long page_size = sysconf(_SC_PAGESIZE); // Typically 4096 bytes
    size_t num_pages = 4;
    size_t alloc_size = num_pages * page_size;

    // 1. Allocate 4 pages of anonymous memory (initially uncommitted/unmapped to physical RAM)
    char *addr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Allocate vector array for mincore (1 byte per page)
    unsigned char vec[num_pages];

    printf("=== MINCORE DEMONSTRATION ===\n");
    printf("Allocated %zu pages (%zu bytes) at address: %p\n\n", num_pages, alloc_size, (void *)addr);

    // 2. Check initial page residency before touching memory
    if (mincore(addr, alloc_size, vec) == -1) {
        perror("mincore failed");
        munmap(addr, alloc_size);
        return 1;
    }

    printf("[BEFORE ACCESS]\n");
    for (size_t i = 0; i < num_pages; i++) {
        printf("  Page %zu: %s (vec[%zu] & 1 = %d)\n",
               i, (vec[i] & 1) ? "IN RAM" : "NOT IN RAM", i, vec[i] & 1);
    }

    // 3. Touch ONLY Page 1 to trigger a page fault and bring it into physical RAM
    printf("\n--> Writing to Page 1 (offset %ld bytes)...\n\n", page_size);
    addr[page_size] = 'A';

    // 4. Check page residency again with mincore
    if (mincore(addr, alloc_size, vec) == -1) {
        perror("mincore failed");
        munmap(addr, alloc_size);
        return 1;
    }

    printf("[AFTER ACCESSING PAGE 1]\n");
    for (size_t i = 0; i < num_pages; i++) {
        printf("  Page %zu: %s (vec[%zu] & 1 = %d)\n",
               i, (vec[i] & 1) ? "IN RAM" : "NOT IN RAM", i, vec[i] & 1);
    }

    // Clean up
    munmap(addr, alloc_size);
    return 0;
}
