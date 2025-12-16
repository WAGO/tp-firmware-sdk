#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#

LIB_OBJS = libmsgtool.o
LIB_FILE = libmsgtool.so

#CFLAGS += -O2 -g -Wall -fpic #-Wsign-compare -Wfloat-equal -Wformat-security #-Werror
CFLAGS += -O2 -g -Wall -fpic -Wfloat-equal -Wformat-security
CPPFLAGS +=
DYNLDFLAGS += -shared

all: $(LIB_FILE)

$(LIB_FILE): $(LIB_OBJS)
	$(CC) $(DYNLDFLAGS) -o $(LIB_FILE) $(LIB_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f $(LIB_OBJS) $(LIB_FILE)

install:
	cp msgtool.h $(DESTDIR)/usr/include
	cp libmsgtool.so $(DESTDIR)/usr/lib

.PHONY: all install clean

# end of development makefile
