#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define PROC_FILE "/proc/myprocfile"

int main() {
    char buf[128];
    int fd;

    // write to /proc file
    fd = open(PROC_FILE, O_WRONLY);
    if (fd < 0) { perror("open"); return 1; }
    write(fd, "Hello kernel!", 13);
    close(fd);

    // read from /proc file
    fd = open(PROC_FILE, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    int n = read(fd, buf, sizeof(buf)-1);
    if (n > 0) buf[n] = '\0';
    printf("Read from kernel: %s\n", buf);
    close(fd);

    return 0;
}
