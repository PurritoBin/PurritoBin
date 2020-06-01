TARGET	=	purrito

DESTDIR	?=

prefix	?= /usr/local
exec_prefix	?= $(prefix)
bindir	?= $(exec_prefix)/bin

CXXFLAGS	+= -std=c++2a -Wall -Wextra -Wpedantic -Wstrict-overflow

# include additional standard directories
# on openbsd we need libuv and we create all static libraries
CXXFLAGS += -I${LOCALBASE}/include -I${LOCALBASE}/include/uSockets
LDFLAGS += -L${LOCALBASE}/lib -l:libssl.a -l:libcrypto.a -l:libpthread.a

# on openbsd we need libuv
ifeq ($(WITH_LIBUV), 1)
CFLAGS	+= -l:libuv.a
endif

LDFLAGS	+= -l:libusockets.a

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
# plus all their dependencies (PurritoBin doesn't need zlib for uWebSockets)
all:
	$(CXX) -DUWS_NO_ZLIB main.cc purrito.cc $(CXXFLAGS) -o $(TARGET) $(LDFLAGS)

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: clean
