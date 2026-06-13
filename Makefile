# Makefile for make!
.POSIX:
.PHONY: install uninstall test m2-check clean

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

OBJS = check.o classify.o command.o condition.o expand.o extensions.o input.o macro.o main.o make.o modtime.o options.o read.o rules.o runtime.o startup.o target.o utils.o
M2_CC = ../stage0-posix/AMD64/bin/M2-Mesoplanet
M2_BINDIR = ../stage0-posix/AMD64/bin
M2LIBC_PATH = ../stage0-posix/M2-Mesoplanet/M2libc
M2_OBJS = check.m2.o extensions.m2.o macro.m2.o main.m2.o make.m2.o modtime.m2.o options.m2.o read.m2.o rules.m2.o runtime.m2.o startup.m2.o target.m2.o utils.m2.o

make: $(OBJS)
	$(CC) $(LDFLAGS) -o make $(OBJS)

$(OBJS): make.h
check.o: make_m2.h
extensions.o: make_m2.h
macro.o: make_m2.h
main.o: make_m2.h
make.o: make_m2.h
modtime.o: make_m2.h
options.o: make_m2.h
read.o: make_m2.h
rules.o: make_m2.h
runtime.o: make_m2.h
startup.o: make_m2.h
target.o: make_m2.h
utils.o: make_m2.h

install: make
	test -d $(DESTDIR)$(BINDIR) || mkdir -p $(DESTDIR)$(BINDIR)
	cp -f make $(DESTDIR)$(BINDIR)/pdpmake
	test -d $(DESTDIR)$(MANDIR)/man1 || mkdir -p $(DESTDIR)$(MANDIR)/man1
	cp -f pdpmake.1 $(DESTDIR)$(MANDIR)/man1/pdpmake.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/pdpmake
	rm -f $(DESTDIR)$(MANDIR)/man1/pdpmake.1

test: make
	@cd testsuite && ./runtest

m2-check: $(M2_OBJS)

check.m2.o: check.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f check.c -o $@

extensions.m2.o: extensions.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f extensions.c -o $@

macro.m2.o: macro.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f macro.c -o $@

main.m2.o: main.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f main.c -o $@

make.m2.o: make.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f make.c -o $@

modtime.m2.o: modtime.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f modtime.c -o $@

options.m2.o: options.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f options.c -o $@

read.m2.o: read.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f read.c -o $@

rules.m2.o: rules.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f rules.c -o $@

runtime.m2.o: runtime.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f runtime.c -o $@

startup.m2.o: startup.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f startup.c -o $@

target.m2.o: target.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f target.c -o $@

utils.m2.o: utils.c make_m2.h
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f utils.c -o $@

clean:
	rm -f $(OBJS) $(M2_OBJS) make
