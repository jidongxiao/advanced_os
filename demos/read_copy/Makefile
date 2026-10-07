obj-m += pagecache_probe.o

KDIR ?= $(shell readlink -f /lib/modules/$$(uname -r)/build)
PWD := $(shell pwd)

CC := gcc
CFLAGS := -O0 -Wall

all: read_copy pagecache_probe.ko data.bin

read_copy: read_copy.c
	$(CC) $(CFLAGS) -o $@ $<

data.bin:
	dd if=/dev/urandom of=data.bin bs=4096 count=1

pagecache_probe.ko: pagecache_probe.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f read_copy data.bin
