KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += lottery.o

all: module app

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app: app.c lottery.h
	$(CC) -Wall -O2 app.c -o app

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f app

.PHONY: all module clean
