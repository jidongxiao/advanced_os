obj-m += misc_ioctl_demo.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -O2 test_ioctl.c -o test_ioctl

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test_ioctl
