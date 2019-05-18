ifdef DEBUG
CFLAGS=-O0 -g -D_DEBUG
else
CFLAGS=-O3 -DNDEBUG
endif

CFLAGS+=-D_GNU_SOURCE -fvisibility=hidden
LDLIBS+=-lcrypto

.PHONY: build rebuild clean

build: lmpc

rebuild: clean build

clean:
	rm -f *.o lmpc

lmpc: lmpc.o config.o server.o

lmpc.c: config.h server.h

config.c: config.h

server.c: server.h

server.h: config.h
