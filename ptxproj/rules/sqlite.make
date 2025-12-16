# -*-makefile-*-
#
# Copyright (C) 2004 by Ladislav Michl
#               2009 by Juergen Beisert <j.beisert@pengtronix.de>
#               2009 by Erwin Rol <erwin@erwinrol.com>
#               2010, 2012 by Marc Kleine-Budde <mkl@pengutronix.de>
#               2011 by Markus Rathgeb <rathgeb.markus@googlemail.com>
#               2016 by Clemens Gruber <clemens.gruber@pqgruber.com>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_SQLITE) += sqlite

define sqlite/expand
$(if $(1),$(if $(filter $(1),0 1 2 3 4 5 6 7 8 9),0)$(1),00)
endef
define sqlite/file-version2
$(word 1,$(1))$(call sqlite/expand,$(word 2,$(1)))$(call sqlite/expand,$(word 3,$(1)))$(call sqlite/expand,$(word 4,$(1)))
endef
define sqlite/file-version
$(call sqlite/file-version2,$(subst ., ,$(strip $(1))))
endef

#
# Paths and names
#
SQLITE_VERSION	:= 3.50.4
SQLITE_MD5	:= d74bbdca4ab1b2bd46d3b3f8dbb0f3db
SQLITE		:= sqlite-autoconf-$(call sqlite/file-version,$(SQLITE_VERSION))
SQLITE_SUFFIX	:= tar.gz
SQLITE_URL	:= https://www.sqlite.org/2025/$(SQLITE).$(SQLITE_SUFFIX)
SQLITE_SOURCE	:= $(SRCDIR)/$(SQLITE).$(SQLITE_SUFFIX)
SQLITE_DIR	:= $(BUILDDIR)/$(SQLITE)
SQLITE_LICENSE	:= public_domain
SQLITE_LICENSE_FILES	:= file://sqlite3.c;startline=35;endline=36;md5=43af35cab122fd0eed4d5469d0507788

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

# sqlite has an unusual config system where some defines are set by the
# configure script, but others can still be defined on the compiler command
# line. Pin down all configure options so that configure_helper.py is pleased,
# but apart from that, set only the defines that we really want (or otherwise
# our CPPFLAGS would explode).
SQLITE_CONF_ENV := \
	$(CROSS_ENV) \
	CCACHE=none \
	CFLAGS="-DzlibVersion=force-syntax-error" \
	CPPFLAGS=" \
	-DSQLITE_ENABLE_COLUMN_METADATA=1 \
	-DSQLITE_ENABLE_FTS3_PARENTHESIS=1 \
	-DSQLITE_ENABLE_UNLOCK_NOTIFY=1 \
	-DSQLITE_DEFAULT_FOREIGN_KEYS=$(call ptx/ifdef, PTXCONF_SQLITE_FOREIGN_KEYS,1,0) \
	-DSQLITE_SOUNDEX=1 \
	"

SQLITE_CONF_TOOL	:= autoconf
SQLITE_CONF_OPT		:= \
	$(CROSS_AUTOCONF_USR) \
	$(GLOBAL_LARGE_FILE_OPTION) \
	--soname=legacy \
	--disable-static \
	--disable-editline \
	--$(call ptx/endis,PTXCONF_SQLITE_READLINE)-readline \
	--$(call ptx/endis,PTXCONF_SQLITE_THREADSAFE)-threadsafe \
	--$(call ptx/endis,PTXCONF_SQLITE_LOAD_EXTENSION)-load-extension \
	--disable-math \
	--enable-fts4 \
	--enable-fts3 \
	--disable-fts5 \
	--enable-rtree \
	--disable-session \
	--disable-debug \
	--disable-static-shell \
	\
	--with-readline-cflags="-I$(SYSROOT)/usr/include"

# linking sqlite3 to libsqlite3.so (--disable-static-shell) is broken without this
SQLITE_MAKE_ENV	:= \
	LDFLAGS=-lm

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/sqlite.targetinstall:
	@$(call targetinfo)

	@$(call install_init, sqlite)
	@$(call install_fixup, sqlite,PRIORITY,optional)
	@$(call install_fixup, sqlite,SECTION,base)
	@$(call install_fixup, sqlite,AUTHOR,"Ladislav Michl <ladis@linux-mips.org>")
	@$(call install_fixup, sqlite,DESCRIPTION,missing)

	@$(call install_lib, sqlite, 0, 0, 0644, libsqlite3)

ifdef PTXCONF_SQLITE_TOOL
	@$(call install_copy, sqlite, 0, 0, 0755, -, /usr/bin/sqlite3)
endif

	@$(call install_finish, sqlite)

	@$(call touch)

# vim: syntax=make