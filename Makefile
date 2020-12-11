TARGET	?=	purrito

DESTDIR ?=
PREFIX ?=	/usr/local
BINDIR ?=	${PREFIX}/bin
MANDIR ?=	${PREFIX}/man

PKG_CONFIG ?=	pkg-config
INSTALL ?=	install

SRC =		src/main.cc
MAN =		man/purrito.1

CXXFLAGS +=	-std=c++2a

LIBS +=		libusockets

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
CXXFLAGS +=	`${PKG_CONFIG} --cflags ${LIBS}`
LDFLAGS +=	`${PKG_CONFIG} --libs ${LIBS}`

all:
	${CXX} ${CXXFLAGS} -DUWS_NO_ZLIB ${SRC} -o ${TARGET} ${LDFLAGS}

install:
	${INSTALL} -d ${DESTDIR}${BINDIR} ${DESTDIR}${MANDIR}/man1
	${INSTALL} -m 0755 ${TARGET} ${DESTDIR}${BINDIR}
	${INSTALL} -m 0644 ${MAN} ${DESTDIR}${MANDIR}/man1

clean:
	rm -f ${TARGET}

.PHONY: all install clean
