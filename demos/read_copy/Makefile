obj-m += pagecache_probe.o

KDIR ?= $(shell readlink -f /lib/modules/$$(uname -r)/build)
PWD := $(shell pwd)

all: data.bin
	$(MAKE) -C $(KDIR) M=$(PWD) modules

data.bin:
	dd if=/dev/urandom of=data.bin bs=4096 count=1

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f data.bin
