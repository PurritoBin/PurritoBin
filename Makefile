TARGET	=	purrito

DESTDIR	?=

CXXFLAGS	+= -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow

PREFIX	=	${DESTDIR}/usr/local/bin

all:
	${CXX} main.cc purrito.cc ${CFLAGS} ${CXXFLAGS} -o ${TARGET}

install: all
	install -m 0755 ${TARGET} ${PREFIX}

clean:
	${RM} ${TARGET}

.PHONY: clean
