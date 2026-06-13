# Makefile for make!
.POSIX:
.PHONY: install uninstall test m2-check clean
.SUFFIXES: .c .m2.o

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

CC = c99
OBJS = check.o classify.o command.o condition.o expand.o extensions.o input.o macro.o main.o make.o modtime.o options.o read.o rules.o runtime.o startup.o target.o utils.o
M2_CC = ../stage0-posix/AMD64/bin/M2-Mesoplanet
M2_BINDIR = ../stage0-posix/AMD64/bin
M2LIBC_PATH = ../stage0-posix/M2-Mesoplanet/M2libc
M2_OBJS = $(OBJS:.o=.m2.o)

make: $(OBJS)
	$(CC) $(LDFLAGS) -o make $(OBJS)

$(OBJS): make_m2.h

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

.PRAGMA: suffix_inference

$(M2_OBJS): make_m2.h

.c.m2.o:
	PATH=$(M2_BINDIR):$$PATH M2LIBC_PATH=$(M2LIBC_PATH) $(M2_CC) -c -f $< -o $@

clean:
	rm -f $(OBJS) $(M2_OBJS) make
