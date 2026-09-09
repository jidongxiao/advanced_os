obj-m += ring_kernel.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all kernel user clean

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user:
	gcc -Wall -Wextra -O2 -o ring_user ring_user.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ring_user
