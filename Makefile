TARGET	?=	purrito

DESTDIR ?=
PREFIX ?=	/usr/local
BINDIR ?=	${PREFIX}/bin
MANDIR ?=	${PREFIX}/man
DATADIR ?=	${PREFIX}/share/purritobin

PKG_CONFIG ?=	pkg-config
INSTALL ?=	install

SRCS =		src/main.cc
HEADERS =	src/purrito.h

MAN =		man/purrito.1

CXXFLAGS +=	-std=c++2a -Wall -Wextra -Wpedantic

LIBS +=		libusockets

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
CXXFLAGS +=	`${PKG_CONFIG} --cflags ${LIBS}`
LDFLAGS +=	`${PKG_CONFIG} --libs ${LIBS}` -lpthread

all: ${TARGET}

${TARGET}: ${SRCS} ${HEADERS}
	${CXX} ${CXXFLAGS} ${SRCS} -o ${TARGET} ${LDFLAGS}

install: all
	${INSTALL} -d "${DESTDIR}${BINDIR}" "${DESTDIR}${DATADIR}" "${DESTDIR}${MANDIR}/man1"
	${INSTALL} -m 0755 ${TARGET} "${DESTDIR}${BINDIR}"
	${INSTALL} -m 0644 ${MAN} "${DESTDIR}${MANDIR}/man1"
	${INSTALL} -m 0644 frontend/paste.html clients/POSIX_shell_client.sh "${DESTDIR}${DATADIR}"

check: test

test: all
	${MAKE} -C tests test

clean:
	rm -f ${TARGET}
	${MAKE} -C tests clean

.PHONY: all install check test clean
