CFLAGS=`xml2-config --cflags`
CFLAGS+=-g
CFLAGS+=-DDEBUG
LDFLAGS=`xml2-config --libs`

LIBFILES=util.o error.o grab.o service.o

all: xpaths

xpaths: xpaths.o ${LIBFILES}

%.o: grab.h %.c

%: %.o
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o xpaths
