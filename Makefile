TARGET	=	purrito

DESTDIR	?=

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

CXXFLAGS	+= -lpthread -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow

all:
	$(CXX) main.cc purrito.cc $(CFLAGS) $(CXXFLAGS) -o $(TARGET)

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: clean
