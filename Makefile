TARGET	=	purrito

DESTDIR	?=

PREFIX ?=	/usr/local/bin

CXXFLAGS	+= -lpthread -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow

all:
	${CXX} main.cc purrito.cc ${CFLAGS} ${CXXFLAGS} -o ${TARGET}

install:
	install -m 0755 ${TARGET} ${DESTDIR}/${PREFIX}

clean:
	${RM} ${TARGET}

.PHONY: clean
