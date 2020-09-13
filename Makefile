TARGET	=	purrito

DESTDIR ?=
LOCALBASE ?=	/usr/local

prefix ?=	/usr/local
exec_prefix ?=	$(prefix)
bindir ?=	$(exec_prefix)/bin

OBJS =	src/main.cc

CXXFLAGS +=	-std=c++2a -Wall -Wextra -Wpedantic -Wstrict-overflow

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
CXXFLAGS +=	-I$(LOCALBASE)/include
LDFLAGS +=	-L$(LOCALBASE)/lib -lcrypto -lssl -lpthread -lusockets

.ifdef USE_STATIC
CXXFLAGS +=	-static
.endif
.ifdef USE_LIBUV
LDFLAGS +=	-luv
.endif

all:
	$(CXX) -DUWS_NO_ZLIB $(OBJS) $(CXXFLAGS) -o $(TARGET) $(LDFLAGS)

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

uninstall:
	rm $(DESTDIR)$(bindir)/$(TARGET)

clean:
	rm $(TARGET)

.PHONY: clean
