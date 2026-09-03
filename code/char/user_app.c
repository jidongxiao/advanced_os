#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/hello_char", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char write_buf[] = "Hello from user space!";
    write(fd, write_buf, strlen(write_buf));

    char read_buf[128];
    int n = read(fd, read_buf, sizeof(read_buf));
    read_buf[n] = '\0';

    printf("Read from kernel: %s\n", read_buf);
    close(fd);
    return 0;
}
