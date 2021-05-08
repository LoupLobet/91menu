VERSION = 1.0
CC = cc
PREFIX = /usr/loca
MANPREFIX = ${PREFIX}/man
PROGNAME = 91menu

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib
FREETYPEINC = ${X11INC}/freetype2
FREETYPELIB = -lfontconfig -lXft
#XINERAMALIBS  = -lXinerama
#XINERAMAFLAGS = -DXINERAMA

INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -L${X11LIB} -lX11 ${FREETYPELIB} #$(XINERAMALIBS)

CPPFLAGS = -DVERSION=\"${VERSION}\" #$(XINERAMAFLAGS)
CFLAGS = -std=c99 -pedantic -Wall -Os ${CPPFLAGS} ${INCS}
LDFLAGS = ${LIBS}

all: clean options 91menu clean_object

options:
	@echo 91menu build options:
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "CC      = ${CC}"

.c.o :
	${CC} -c ${CFLAGS} $<

91menu: 91menu.o drw.o utf8.o util.o
	${CC} -o $@ 91menu.o drw.o utf8.o util.o ${LDFLAGS}

clean_object:
	rm -f 91menu.o drw.o utf8.o util.o
clean:
	rm -f 91menu 91menu.o drw.o utf8.o util.o *.core

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -r 91menu ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/91menu
	sed "s/VERSION/${VERSION}/g" <91menu.1 >${DESTDIR}${MANPREFIX}/man1/91menu.1


uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/91menu
	rm -f ${DESTDIR}${MANPREFIX}/man1/91menu.1

.PHONY: all options clean install uninstall