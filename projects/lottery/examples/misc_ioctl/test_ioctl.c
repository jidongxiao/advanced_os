#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEMO_IOC_MAGIC 'k'
#define DEMO_IOC_SET_VAL _IOW(DEMO_IOC_MAGIC, 1, int)
#define DEMO_IOC_GET_VAL _IOR(DEMO_IOC_MAGIC, 2, int)

int main(void)
{
    int fd, val;

    fd = open("/dev/demo_dev", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/demo_dev");
        return EXIT_FAILURE;
    }

    /* 1. Get initial value */
    if (ioctl(fd, DEMO_IOC_GET_VAL, &val) < 0) {
        perror("IOCTL GET failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[USER] Read initial kernel value: %d\n", val);

    /* 2. Set new value */
    val = 100;
    printf("[USER] Setting kernel value to: %d\n", val);
    if (ioctl(fd, DEMO_IOC_SET_VAL, &val) < 0) {
        perror("IOCTL SET failed");
        close(fd);
        return EXIT_FAILURE;
    }

    /* 3. Read back updated value */
    if (ioctl(fd, DEMO_IOC_GET_VAL, &val) < 0) {
        perror("IOCTL GET failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[USER] Read updated kernel value: %d\n", val);

    close(fd);
    return EXIT_SUCCESS;
}
