#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#


ifndef PTXDIST_SYSROOT_TARGET
#$(error Environment variable PTXDIST_SYSROOT_TARGET not set properly!)
endif

#CFLAGS += -O2 -g -Wall #-Wsign-compare -Wfloat-equal -Wformat-security #-Werror
#CPPFLAGS +=
#LDFLAGS +=

GLIB_CFLAGS  = -I$(PTXDIST_SYSROOT_TARGET)/usr/include/glib-2.0
GLIB_CFLAGS += -I$(PTXDIST_SYSROOT_TARGET)/usr/lib/glib-2.0/include
GLIB_LDFLAGS = -lglib-2.0
override LDFLAGS += -L$(PTXDIST_SYSROOT_TARGET)/lib -ludev -lmsgtool -lctcommon $(GLIB_LDFLAGS)
override CFLAGS += $(GLIB_CFLAGS)
override CFLAGS+=-O0 -ggdb3


all: get_operatinghours

get_operatinghours: get_operatinghours.o
	$(CC) $(LDFLAGS) -o $@ get_operatinghours.o $(LDLIBS$(LDLIBS-$(@)))


clean:
	rm -f get_operatinghours *.gdb *.o

install:


.PHONY: all install clean

# end of development makefile
