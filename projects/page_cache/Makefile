obj-m += pagecacheTest.o

# Dynamically target the running kernel's build directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CC := gcc
CFLAGS := -O0 -Wall

all: readFile pagecacheTest.ko data.bin

readFile: readFile.c
	$(CC) $(CFLAGS) -o $@ $<

data.bin:
	dd if=/dev/urandom of=$@ bs=4096 count=1

pagecacheTest.ko: pagecacheTest.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f readFile data.bin
