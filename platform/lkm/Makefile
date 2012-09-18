obj-m:= konohadev.o
konohadev-objs := \
	../../src/konoha/minikonoha.o\
	../../module/msgc/msgc.o\
	../../src/sugar/sugar.o\
	../../src/logger/logger.o\
	../../src/vm/compiler.o\
	qsort.o\
	lkm.o


KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
VERBOSE=0
EXTRA_CFLAGS := -DPLATAPIFORM_KERNEL
EXTRA_CFLAGS += -Wno-declaration-after-statement
EXTRA_CFLAGS += -I$(SRCDIR)/include -I$(SRCDIR)/platform/lkm/

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules KBUILD_VERBOSE=$(VERBOSE)

install-lkm:
	/sbin/insmod ./konohadev.ko  || exit 1

install:install-lkm
	rm -f /dev/konoha
	mknod /dev/konoha c $(shell awk "\$$2==\"konohadev\" {print \$$1}" /proc/devices) 0
	chgrp $(shell whoami) /dev/konoha
	chmod 666   /dev/konoha

uninstall:
	/sbin/rmmod konohadev

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ../../src/*/.*.o.cmd ../../src/*/*.o
	rm -f ../../module/*/.*.o.cmd ../../module/*/*.o


