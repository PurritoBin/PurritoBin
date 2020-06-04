TARGET	=	purrito

DESTDIR	?=

prefix	?= /usr/local
exec_prefix	?= $(prefix)
bindir	?= $(exec_prefix)/bin

OBJS	= src/main.cc src/purrito.cc

CXXFLAGS	+= -std=c++2a -Wall -Wextra -Wpedantic -Wstrict-overflow

CXXFLAGS += -I${LOCALBASE}/include -I${LOCALBASE}/include/uSockets

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
.ifdef USE_STATIC
LDFLAGS += -L$(LOCALBASE)/lib -l:libcrypto.a -l:libssl.a -l:libpthread.a
# on openbsd we need libuv
.ifdef USE_LIBUV
LDFLAGS	+= -l:libuv.a
.endif
LDFLAGS	+= -l:libusockets.a
.else
LDFLAGS	+= -L$(LOCALBASE)/lib -lssl -lpthread
.ifdef USE_LIBUV
LDFLAGS	+= -luv
.endif
LDFLAGS	+= -lusockets
.endif

all:
	$(CXX) -DUWS_NO_ZLIB $(OBJS) $(CXXFLAGS) -o $(TARGET) $(LDFLAGS)

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

clean:
	rm $(TARGET)

.PHONY: clean
