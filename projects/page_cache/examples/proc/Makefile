obj-m += my_proc.o

KDIR ?= $(shell readlink -f /lib/modules/$$(uname -r)/build)
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
