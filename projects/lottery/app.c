#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "lottery.h"

int lottery_fd;

int lucas(int n) {
    if (n == 0) return 2;
    if (n == 1) return 1;
    return (lucas(n - 1) + lucas(n - 2));
}

double getMilliSeconds(void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (double)now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}

int register_process(struct lottery_struct lottery_info) {
    int ret = ioctl(lottery_fd, LOTTERY_REGISTER, &lottery_info);
    if (ret != 0) {
        printf("ioctl fails\n");
        return -1;
    }
    return 0;
}

int unregister_process(struct lottery_struct lottery_info) {
    int ret = ioctl(lottery_fd, LOTTERY_UNREGISTER, &lottery_info);
    if (ret != 0) {
        printf("ioctl fails\n");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int ret;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <tickets> <n>\n", argv[0]);
        exit(1);
    }

    unsigned long pid = getpid();
    unsigned long tickets = atoi(argv[1]);
    unsigned long n = atoi(argv[2]);
    struct lottery_struct tmp;

    lottery_fd = open("/dev/lottery", O_RDWR);
    if (lottery_fd <= 0) {
        printf("lottery: failed to open /dev/lottery device\n");
        return -errno;
    }

    tmp.pid = pid;
    tmp.tickets = tickets;
    ret = register_process(tmp);
    if (ret != 0) {
        printf("%lu: unable to register.\n", pid);
        close(lottery_fd);
        exit(1);
    }

    double start_time = getMilliSeconds();
    lucas(n);
    double computing_time = getMilliSeconds() - start_time;
    
    printf("pid %lu, with %lu tickets: computing lucas(%lu) took %4.2lf seconds.\n", 
            pid, tickets, n, computing_time / 1000.0);

    ret = unregister_process(tmp);
    if (ret != 0) {
        printf("%lu: unable to unregister.\n", pid);
    }

    close(lottery_fd);
    return 0;
}
