# Rules for making the NTFS driver.
obj-m := ntfs.o

ntfs-objs := aops.o attrib.o collate.o compress.o debug.o dir.o file.o \
	  index.o inode.o mft.o mst.o namei.o runlist.o super.o sysctl.o \
	  unistr.o upcase.o

# Location of the kernel build system
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

ccflags-y := -DNTFS_VERSION=\"2.1.30\"
