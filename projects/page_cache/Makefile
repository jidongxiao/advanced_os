obj-m += pagecacheTest.o

KDIR ?= $(shell readlink -f /lib/modules/$$(uname -r)/build)
PWD := $(shell pwd)

CC := gcc
CFLAGS := -O0 -Wall

all: readFile pagecacheTest.ko data.bin

readFile: readFile.c
	$(CC) $(CFLAGS) -o $@ $<

data.bin:
	dd if=/dev/urandom of=data.bin bs=4096 count=1

pagecacheTest.ko: pagecacheTest.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f readFile data.bin
