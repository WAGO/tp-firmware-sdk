# -*-makefile-*-
#
# Copyright (C) 2014 by Michael Olbrich <m.olbrich@pengutronix.de>
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
LIBEPOXY_VERSION	:= 1.5.3
LIBEPOXY_MD5		:= e2845de8d2782b2d31c01ae8d7cd4cbb
LIBEPOXY		:= libepoxy-$(LIBEPOXY_VERSION)
LIBEPOXY_SUFFIX		:= tar.xz
LIBEPOXY_URL		:= https://github.com/anholt/libepoxy/releases/download/$(LIBEPOXY_VERSION)/$(LIBEPOXY).$(LIBEPOXY_SUFFIX)
LIBEPOXY_SOURCE		:= $(SRCDIR)/$(LIBEPOXY).$(LIBEPOXY_SUFFIX)
LIBEPOXY_DIR		:= $(BUILDDIR)/$(LIBEPOXY)
LIBEPOXY_LICENSE	:= MIT

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
LIBEPOXY_CONF_TOOL	:= meson
ifeq ($(PTXCONF_CONFIGFILE_VERSION), "2020.08.0")
LIBEPOXY_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Ddocs=false \
	-Degl=$(call ptx/yesno,PTXCONF_LIBEPOXY_EGL) \
	-Dglx=$(call ptx/yesno,PTXCONF_LIBEPOXY_GLX) \
	-Dtests=false \
	-Dx11=$(call ptx/truefalse,PTXCONF_LIBEPOXY_GLX)
else
LIBEPOXY_CONF_OPT	:= \
	$(CROSS_MESON_USR) \
	-Ddocs=false \
	-Degl=$(call ptx/ifdef,PTXCONF_LIBEPOXY_EGL, yes, no) \
	-Dglx=$(call ptx/ifdef,PTXCONF_LIBEPOXY_GLX, yes, no) \
	-Dtests=false \
	-Dx11=$(call ptx/truefalse,PTXCONF_LIBEPOXY_GLX)
endif

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
