#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#
GLIB_CFLAGS  = -I$(PTXDIST_SYSROOT_TARGET)/usr/include/glib-2.0
GLIB_CFLAGS += -I$(PTXDIST_SYSROOT_TARGET)/usr/lib/glib-2.0/include
GLIB_LDFLAGS = -lglib-2.0
override LDFLAGS += -L$(PTXDIST_SYSROOT_TARGET)/lib -ludev -lmsgtool -lctcommon $(GLIB_LDFLAGS)
override CFLAGS += $(GLIB_CFLAGS)
override CFLAGS+=-O0 -ggdb3

all: config_display

config_display: config_display.o
	$(CC) $(LDFLAGS) -o $@ config_display.o $(LDLIBS$(LDLIBS-$(@)))


clean:
	rm -f config_display *.gdb *.o


install:


.PHONY: all install clean

# end of development makefile
