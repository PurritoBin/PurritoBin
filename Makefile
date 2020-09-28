TARGET	?=	purrito

DESTDIR ?=
PKG_CONFIG ?=	pkg-config
INSTALL ?=	install

PREFIX ?=	/usr/local
BINDIR ?=	${PREFIX}/bin

OBJS =		src/main.cc

CXXFLAGS +=	-std=c++2a -Wall -Wextra -Wpedantic -Wstrict-overflow

LIBS +=		usockets libssl

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
CXXFLAGS +=	`${PKG_CONFIG} --cflags ${LIBS}`
LDFLAGS +=	`${PKG_CONFIG} --libs ${LIBS}`

all:
	${CXX} -DUWS_NO_ZLIB ${OBJS} ${CXXFLAGS} -o ${TARGET} ${LDFLAGS}

install:
	${INSTALL} -d ${DESTDIR}${BINDIR}
	${INSTALL} -m 0755 ${TARGET} ${DESTDIR}${BINDIR}/${TARGET}

uninstall:
	rm ${DESTDIR}${bindir}/${TARGET}

clean:
	rm ${TARGET}

.PHONY: clean
