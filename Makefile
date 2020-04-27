TARGET	=	purrito

DESTDIR	?=
PREFIX ?= /usr/local/bin

CXXFLAGS	+= -lpthread -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow

INSTALLDIR	=	${DESTDIR}${PREFIX}

all:
	${CXX} main.cc purrito.cc ${CFLAGS} ${CXXFLAGS} -o ${TARGET}

install: all
	install -m 0755 ${TARGET} ${INSTALLDIR}

clean:
	${RM} ${TARGET}

.PHONY: clean
