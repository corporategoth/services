# Makefile for Services


include Makefile.inc

VERSION = 3.0.9

########################## Configuration section ##########################


# Compilation options:
#		none.
#
# If you change this value, REMEMBER to "make clean", or you may come out
# with a confused executable!

CDEFS =


######################## End configuration section ########################


# I lied: you may need to change this depending on what flags your `cp'
# takes.  This should be the command to copy a directory and all its
# files to another directory.  (This is correct for Linux.)

CP_ALL = cp -dpr


CFLAGS = -g $(CDEFS) $(EXTRA_CFLAGS)


OBJS =	channels.o chanserv.o helpserv.o main.o memoserv.o misc.o \
	nickserv.o operserv.o process.o send.o sockutil.o users.o
SRCS =	channels.c chanserv.c helpserv.c main.c memoserv.c misc.c \
	nickserv.c operserv.c process.c send.c sockutil.c users.c


all: services

clean:
	rm -f *.o services listnicks listchans version.h.old

spotless: clean
	rm -f config.cache sysconf.h Makefile.inc

install: all
	$(INSTALL) services $(BINDEST)/services
	@if [ -x upaccess ]; then \
		$(INSTALL) upaccess $(BINDEST)/upaccess \
	fi
	$(INSTALL) up
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
	@echo
	@echo '#****************************************************#'
	@echo '# If you are upgrading from version 3.0.3 or before, #'
	@echo '# you will need to type "make upaccess" (no quotes). #'
	@echo '# ------------> ONLY EVER DO THIS ONCE <------------ #'
	@echo '#****************************************************#'

.c.o:
	$(CC) $(CFLAGS) -c $<

upaccess:
	$(CC) $(CFLAGS) -o upaccess upaccess.c
	@echo
	@echo '#************************************************#'
	@echo '# READ THE upaccess.c FILE BEFORE USING upaccess #'
	@echo '# If you've upaccess'd before, DONT DO IT AGAIN! #'
	@echo '#************************************************#'

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
	sh version.sh $(VERSION)
