obj-m += get_cache_page.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: get_cache_page.ko data.bin

get_cache_page.ko: get_cache_page.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

data.bin:
	dd if=/dev/urandom of=data.bin bs=4096 count=1

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f data.bin
