ARCH := $(shell uname -m)

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

ifeq ($(ARCH),x86_64)

USER_SRC := privileged_user_x86_64.c
KERNEL_SRC := privileged_x86_64.o

else ifeq ($(ARCH),aarch64)

USER_SRC := privileged_user_aarch64.c
KERNEL_SRC := privileged_aarch64.o

else

$(error Unsupported architecture: $(ARCH))

endif

obj-m += privileged.o
privileged-objs := $(KERNEL_SRC)

.PHONY: all kernel user clean

all: kernel user

kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user:
	gcc -Wall -Wextra -O2 -o privileged_user $(USER_SRC)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f privileged_user
