# -*-makefile-*-
#
# Copyright (C) 2014 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBEPOXY) += libepoxy

#
# Paths and names
#
LIBEPOXY_VERSION	:= 1.2
LIBEPOXY_MD5		:= 6524c86e1ad297881024748634089a6e
LIBEPOXY		:= libepoxy-$(LIBEPOXY_VERSION)
LIBEPOXY_SUFFIX		:= tar.bz2
LIBEPOXY_URL		:= https://github.com/anholt/libepoxy.git;tags/$(LIBEPOXY).$(LIBEPOXY_SUFFIX)
LIBEPOXY_SOURCE		:= $(SRCDIR)/$(LIBEPOXY).$(LIBEPOXY_SUFFIX)
LIBEPOXY_DIR		:= $(BUILDDIR)/$(LIBEPOXY)
LIBEPOXY_LICENSE	:= MIT

#-----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

LIBEPOXY_CONF_ENV	:= \
	$(CROSS_ENV) \
	ac_cv_prog_PYTHON=python \
	PKG_CONFIG_PATH=$(PTXDIST_SYSROOT_TARGET)/usr/lib/pkgconfig
#
# autoconf
#
LIBEPOXY_CONF_TOOL	:= autoconf
LIBEPOXY_CONF_OPT	:= \
	$(CROSS_AUTOCONF_USR) \
	--disable-strict-compilation \
	$(GLOBAL_LARGE_FILE_OPTION)
	
$(STATEDIR)/libepoxy.prepare: $(bootpc_prepare_deps_default)
	@$(call targetinfo)
	cd $(LIBEPOXY_DIR) && autoreconf -v --install && \
	$(LIBEPOXY_CONF_ENV) CC=$(CROSS_CC) ./configure $(LIBEPOXY_CONF_OPT) \
	PKG_CONFIG=$(PTXDIST_SYSROOT_CROSS)/bin \
	PKG_CONFIG_PATH=$(PTXDIST_SYSROOT_TARGET)/usr/lib/pkgconfig \
	--with-sysroot=$(PTXDIST_SYSROOT_TARGET) \
	X11_CFLAGS="-I$(PTXDIST_SYSROOT_TARGET)/usr/include -I$(PTXDIST_SYSROOT_TARGET)/usr/include/xorg -I$(PTXDIST_SYSROOT_TARGET)/usr/include/X11" \
	X11_LIBS="-L$(PTXDIST_SYSROOT_TARGET)/usr/lib "
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libepoxy.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libepoxy)
	@$(call install_fixup, libepoxy,PRIORITY,optional)
	@$(call install_fixup, libepoxy,SECTION,base)
	@$(call install_fixup, libepoxy,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, libepoxy,DESCRIPTION,missing)

	@$(call install_lib, libepoxy, 0, 0, 0644, libepoxy)

	@$(call install_finish, libepoxy)

	@$(call touch)

# vim: syntax=make
