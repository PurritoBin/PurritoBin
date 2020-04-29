TARGET	=	purrito

DESTDIR	?=

prefix	?= /usr/local
exec_prefix	?= $(prefix)
bindir	?= $(exec_prefix)/bin

CXXFLAGS	+= -lpthread -std=c++17 -Wall -Wextra -Wpedantic -Wstrict-overflow

# requirements
# uwebsockets: https://github.com/uNetworking/uWebSockets
# uSockets   : https://github.com/uNetworking/uSockets
# plus all their dependencies (PurritoBin doesn't need zlib for uWebSockets)
all:
	$(CXX) -DUWS_NO_ZLIB -lusockets main.cc purrito.cc $(CFLAGS) $(CXXFLAGS) -o $(TARGET)

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: clean
