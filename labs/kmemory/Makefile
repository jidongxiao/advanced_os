obj-m += kmemory.o
obj-m += kernel_read.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: kernel_modules user_read

kernel_modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user_read:
	gcc -Wall -Wextra -O2 -o user_read user_read.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f user_read
