#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE "/dev/ioctl_example"

#define MAJOR_NUM 240
#define IOCTL_HELLO _IO(MAJOR_NUM, 0)
#define IOCTL_SET_VALUE _IOW(MAJOR_NUM, 1, int)
#define IOCTL_GET_VALUE _IOR(MAJOR_NUM, 2, int)

int main()
{
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // send hello
    ioctl(fd, IOCTL_HELLO);

    // set value
    int val = 42;
    ioctl(fd, IOCTL_SET_VALUE, &val);

    // get value
    int read_val = 0;
    ioctl(fd, IOCTL_GET_VALUE, &read_val);
    printf("Read value from kernel: %d\n", read_val);

    close(fd);
    return 0;
}
