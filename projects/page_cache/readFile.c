#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_NAME "data.bin"
#define FILE_SIZE 4096
#define PAGE_SIZE 4096

int main(void)
{
    int fd;
    char *buffer;
    ssize_t n;

    printf("PID: %d\n", getpid());

    fd = open(FILE_NAME, O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    /*
     * Allocate a page-aligned user buffer.
     */
    if (posix_memalign((void **)&buffer, PAGE_SIZE, FILE_SIZE) != 0) {
        perror("posix_memalign");
        close(fd);
        return 1;
    }

    n = read(fd, buffer, FILE_SIZE);

    if (n != FILE_SIZE) {
        perror("read");
        free(buffer);
        close(fd);
        return 1;
    }

    printf("Read %zd bytes\n", n);
    printf("User buffer virtual address: %p\n", (void *)buffer);

    printf("First 16 bytes:");

    for (int i = 0; i < 16; i++)
        printf(" %02x", (unsigned char)buffer[i]);

    printf("\n");

    printf("Waiting for kernel-module test...\n");
    fflush(stdout);
    pause();

    free(buffer);
    close(fd);

    return 0;
}
