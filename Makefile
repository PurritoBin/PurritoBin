TARGET	?=	purrito

DESTDIR ?=
LOCALBASE ?=	/usr/local
PKG_CONFIG ?=	pkg-config
INSTALL ?=	install

prefix ?=	/usr/local
bindir ?=	${prefix}/bin

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
	${INSTALL} -d ${DESTDIR}${bindir}
	${INSTALL} -m 0755 ${TARGET} ${DESTDIR}${bindir}/${TARGET}

uninstall:
	rm ${DESTDIR}${bindir}/${TARGET}

clean:
	rm ${TARGET}

.PHONY: clean
