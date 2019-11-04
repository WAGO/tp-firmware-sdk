#
# Simple development makefile
#

#
# add more flags here,
# but don't remove the "+="
#

GLIB_CFLAGS  = -I$(PTXDIST_SYSROOT_TARGET)/usr/include/glib-2.0
GLIB_CFLAGS += -I$(PTXDIST_SYSROOT_TARGET)/usr/lib/glib-2.0/include
GLIB_CFLAGS += -O2 -g -Wall #-Wsign-compare -Wfloat-equal -Wformat-security -D_REENTRANT #-Werror
GLIB_LDFLAGS = -lglib-2.0
override LDFLAGS += -L$(PTXDIST_SYSROOT_TARGET)/lib -ludev -lpthread -lmsgtool -lrt -lm -lctcommon $(GLIB_LDFLAGS)
override CFLAGS += $(GLIB_CFLAGS)
override CFLAGS+=-O0 -ggdb3


all: gesture_control_cap

gesture_control_cap: gesture_control_cap.o
	$(CC) $(LDFLAGS) -o $@ gesture_control_cap.o $(LDLIBS$(LDLIBS-$(@)))


clean:
	rm -f gesture_control_cap *.gdb *.o

install:


.PHONY: all install clean

# end of development makefile
