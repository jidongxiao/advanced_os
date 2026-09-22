obj-m += signal_control_demo.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -O2 target_app.c -o target_app

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f target_app
