obj-m += pagecache_probe.o

KDIR := /home/xiaoj8/kernels/linux-7.0
PWD := $(shell pwd)

all: data.bin
	make -C $(KDIR) M=$(PWD) modules

data.bin:
	dd if=/dev/urandom of=data.bin bs=4096 count=1

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f data.bin
