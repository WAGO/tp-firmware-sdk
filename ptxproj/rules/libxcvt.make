# -*-makefile-*-
#
# Copyright (C) 2022 by Lucas Stach <l.stach@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBXCVT) += libxcvt

#
# Paths and names
#
LIBXCVT_VERSION		:= 0.1.2
LIBXCVT_MD5		:= b553fdb6024c5a137ff925bf4c337724
LIBXCVT			:= libxcvt-$(LIBXCVT_VERSION)
LIBXCVT_SUFFIX		:= tar.xz
LIBXCVT_URL		:= https://www.x.org/pub/individual/lib/$(LIBXCVT).$(LIBXCVT_SUFFIX)
LIBXCVT_SOURCE		:= $(SRCDIR)/$(LIBXCVT).$(LIBXCVT_SUFFIX)
LIBXCVT_DIR		:= $(BUILDDIR)/$(LIBXCVT)
LIBXCVT_LICENSE		:= MIT
LIBXCVT_LICENSE_FILES	:= \
	file://COPYING;md5=129947a06984d6faa6f9a9788fa2a03f

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
#
# meson
#
LIBXCVT_CONF_TOOL	:= meson

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libxcvt.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libxcvt)
	@$(call install_fixup, libxcvt,PRIORITY,optional)
	@$(call install_fixup, libxcvt,SECTION,base)
	@$(call install_fixup, libxcvt,AUTHOR,"Lucas Stach <l.stach@pengutronix.de>")
	@$(call install_fixup, libxcvt,DESCRIPTION,missing)

	@$(call install_lib, libxcvt, 0, 0, 0644, libxcvt)

ifdef PTXCONF_LIBXCVT_TOOL
	@$(call install_copy, libxcvt, 0, 0, 0755, -, /usr/bin/cvt)
endif

	@$(call install_finish, libxcvt)

	@$(call touch)

# vim: syntax=make
