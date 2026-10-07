#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_NAME "data.bin"
#define FILE_SIZE 4096

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
     * malloc() gives us an anonymous user-space buffer.
     */
    buffer = malloc(FILE_SIZE);

    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    /*
     * Ordinary buffered read().
     *
     * The kernel reads the file through the page cache
     * and copies the data into our user buffer.
     */
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

    printf("\nPress Enter to exit...\n");
    getchar();

    free(buffer);
    close(fd);

    return 0;
}
