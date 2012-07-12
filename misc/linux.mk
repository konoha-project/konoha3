# Makefile for linux
# Maintainer:   Masahiro Ide
# Last Change:  2012 June 5

CC ?= gcc
CFLAGS ?= -g -O2 -Wall -fPIC -I./include
#CFLAGS ?= -g3 -O0 -Wall -fPIC -I./include -DK_USING_DEBUG

LDLIBS ?= -ldl
STRIP = strip
DESTDIR ?= /usr/local
konoha = konoha
version = 2.0
PREFIX = $(DESTDIR)
dir    = build

objs = \
	$(dir)/konoha2.o \
	$(dir)/gc.o \
	$(dir)/logger.o \
	$(dir)/sugar.o \
	$(dir)/asm.o

svnversion_exists := $(shell which svnversion)
define compile_with_revision
	$(CC) -DK_REVISION=$(shell svn info 2>&1 | python -c 'import sys, re;t = sys.stdin.read(); print re.search("Revision: (\d+)", t).groups()[0];')
endef

define compile
	$(CC) -DK_REVISION=$(shell $(svnversion_exists) -n ./)
endef

MYCC =$(if $(svnversion_exists) ,$(compile_with_revision), $(compile))

.PHONY: all
all: $(dir)/$(konoha) $(packages)

$(dir)/$(konoha) : src/tool/command.c $(dir)/lib$(konoha).so
	$(MYCC) $(CFLAGS) -o $@ src/tool/command.c -L./$(dir) -l$(konoha) $(LDLIBS)
	$(STRIP) $@

$(dir)/lib$(konoha).so : $(objs)
	$(MYCC) $(CFLAGS) -shared -o $@ $^ -L./$(dir) $(LDLIBS)

## object files
$(dir)/konoha2.o : src/konoha/konoha2.c
	$(MYCC) $(CFLAGS) -c $^ -o $@

$(dir)/gc.o : src/gc/gc.c
	$(MYCC) $(CFLAGS) -c $^ -o $@

$(dir)/logger.o : src/logger/logger.c
	$(MYCC) $(CFLAGS) -c $^ -o $@

$(dir)/sugar.o : src/sugar/sugar.c
	$(MYCC) $(CFLAGS) -c $^ -o $@

$(dir)/asm.o : src/vm/asm.c
	$(MYCC) $(CFLAGS) -c $^ -o $@

## install
#.PHONY: install
#install:
#	bash $(dir)/uninstall.sh $(konoha) $(PREFIX)
#	bash $(dir)/install.sh $(konoha) $(PREFIX)
#
### uninstall
#.PHONY: uninstall
#uninstall:
#	bash $(dir)/uninstall.sh $(konoha) $(PREFIX)

## clean
.PHONY: clean
clean:
	$(RM) -rf $(dir)/*.so $(dir)/*.o $(dir)/$(konoha)
