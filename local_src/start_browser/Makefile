#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#

#CFLAGS += $(shell $(PKG_CONFIG) --cflags wago_dbus)
#LDFLAGS += $(shell $(PKG_CONFIG) --libs wago_dbus)

GLIB_CFLAGS  = -I$(PTXDIST_SYSROOT_TARGET)/usr/include/glib-2.0
GLIB_CFLAGS += -I$(PTXDIST_SYSROOT_TARGET)/usr/lib/glib-2.0/include
GLIB_CFLAGS += -O2 -g -Wall #-Wsign-compare -Wfloat-equal -Wformat-security -D_REENTRANT #-Werror
GLIB_LDFLAGS = -lglib-2.0
override LDFLAGS += -L$(PTXDIST_SYSROOT_TARGET)/lib -ludev -lmsgtool -lctcommon $(GLIB_LDFLAGS)
override CFLAGS += $(GLIB_CFLAGS)
override CFLAGS+=-O0 -ggdb3

all: start_browser

start_browser: start_browser.o
	$(CC) $(LDFLAGS) -o $@ start_browser.o $(LDLIBS$(LDLIBS-$(@)))

clean:
	rm -f start_browser *.gdb *.o

install:

.PHONY: all install clean

# end of development makefile
