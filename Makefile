# Makefile for Services


include Makefile.inc


########################## Configuration section ##########################


# Compilation options:
#	-DSKELETON	Compile a "skeleton" version of Services, with only
#			    OperServ enabled
#	-DREADONLY	Compile a normal version of Services, but with
#			    database modification disabled ("backup" Services)
# Note that SKELETON and READONLY are mutually exclusive.
#
# If you change this value, REMEMBER to "make clean", or you may come out
# with a confused executable!

CDEFS =


######################## End configuration section ########################


# I lied: you may need to change this depending on what flags your `cp'
# takes.  This should be the command to copy a directory and all its
# files to another directory.  (This is correct for Linux.)

CP_ALL = cp -dpr


CFLAGS = $(CDEFS) $(EXTRA_CFLAGS)


OBJS =	channels.o chanserv.o helpserv.o main.o memoserv.o misc.o \
	nickserv.o operserv.o process.o send.o sockutil.o users.o
SRCS =	channels.c chanserv.c helpserv.c main.c memoserv.c misc.c \
	nickserv.c operserv.c process.c send.c sockutil.c users.c


all: services

test: services do-test

do-test:
	make -C test test-services DATA_DIR=`pwd`/test/data
	test/test-services $(TEST_OPTIONS) 0 ./services

clean:
	rm -f *.o services listnicks listchans version.h.old

spotless: clean
	rm -f config.cache sysconf.h Makefile.inc

install: all
	$(INSTALL) services $(BINDEST)/services
	rm -f $(BINDEST)/listnicks $(BINDEST)/listchans
	ln $(BINDEST)/services $(BINDEST)/listnicks
	ln $(BINDEST)/services $(BINDEST)/listchans

install-data:
	$(CP_ALL) data $(DATDEST)
	@if [ "$(RUNGROUP)" ] ; then \
		echo chgrp -R $(RUNGROUP) $(DATDEST) ; \
		chgrp -R $(RUNGROUP) $(DATDEST) ; \
		echo chmod -R g+rw $(DATDEST) ; \
		chmod -R g+rw $(DATDEST) ; \
		echo chmod g+xs \`find $(DATDEST) -type d -print\` ; \
		chmod g+xs `find $(DATDEST) -type d -print` ; \
	fi


########


services: version.h $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@


.c.o:
	$(CC) $(CFLAGS) -c $<

channels.o: channels.c services.h
chanserv.o: chanserv.c cs-help.c services.h
helpserv.o: helpserv.c services.h
main.o: main.c services.h
memoserv.o: memoserv.c ms-help.c services.h
misc.o: misc.c services.h
nickserv.o: nickserv.c ns-help.c services.h
operserv.o: operserv.c os-help.c services.h
process.o: process.c services.h version.h
send.o: send.c services.h
sockutil.o: sockutil.c services.h
users.o: users.c services.h

services.h: config.h extern.h
	touch $@

config.h: sysconf.h
	touch $@

version.h: services.h $(SRCS)
	sh version.sh