# Makefile for Services test suite

ifndef DATA_DIR
DATA_DIR=/usr/local/src/services/test/data
endif

OBJ = test-main.o test-misc.o \
	users.o channels.o

CC=	cc
CFLAGS=	-O6 -DDATA_DIR=\"$(DATA_DIR)\"


all: test-services

test-services: $(OBJ) ../sockutil.o
	$(CC) -o $@ $(OBJ) ../sockutil.o
	strip $@

clean:
	rm -f *.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

test-main.o: test-main.c test.h test-list.c
test-misc.o: test-misc.c test.h

users.o: users.c test.h
channels.o: channels.c test.h

test-list.c: test-list.h
	touch $@
