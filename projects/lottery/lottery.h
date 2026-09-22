#ifndef LOTTERY_H
#define LOTTERY_H

#include <linux/ioctl.h>

struct lottery_struct {
    unsigned long pid;
    unsigned long tickets;
};

#define LOTTERY_MAGIC 'L'
#define LOTTERY_REGISTER   _IOW(LOTTERY_MAGIC, 1, struct lottery_struct)
#define LOTTERY_UNREGISTER _IOW(LOTTERY_MAGIC, 2, struct lottery_struct)

#endif
