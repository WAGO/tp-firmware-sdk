# -*-makefile-*-
#
# Copyright (C) 2018 by Ralf Gliese <gliese@elrest.de>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_UTIL_MACROS) += xorg-util-macros

#
# Paths and names
#
XORG_UTIL_MACROS_VERSION	:= 1.19.2
XORG_UTIL_MACROS_MD5		:= 58edef899364f78fbde9479ded20211e
XORG_UTIL_MACROS			:= util-macros-$(XORG_UTIL_MACROS_VERSION)
XORG_UTIL_MACROS_SUFFIX		:= tar.bz2
#ftp://ftp.x.org/pub/individual/util/util-macros-1.19.2.tar.bz2
XORG_UTIL_MACROS_URL		:= ftp://ftp.x.org/pub/individual/util/$(XORG_UTIL_MACROS).$(XORG_UTIL_MACROS_SUFFIX)
XORG_UTIL_MACROS_SOURCE		:= $(SRCDIR)/$(XORG_UTIL_MACROS).$(XORG_UTIL_MACROS_SUFFIX)
XORG_UTIL_MACROS_DIR		:= $(BUILDDIR)/$(XORG_UTIL_MACROS)
XORG_UTIL_MACROS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XORG_UTIL_MACROS_PATH		:= PATH=$(CROSS_PATH)
XORG_UTIL_MACROS_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XORG_UTIL_MACROS_CONF_TOOL	:= autoconf

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-util-macros.compile:
	@$(call targetinfo)
	@cd $(XORG_UTIL_MACROS_DIR) && make
	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-util-macros.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xorg-util-macros)
	@$(call install_fixup, xorg-util-macros,PRIORITY,optional)
	@$(call install_fixup, xorg-util-macros,SECTION,base)
	@$(call install_fixup, xorg-util-macros,AUTHOR,"Ralf Gliese <gliese@elrest.de>")
	@$(call install_fixup, xorg-util-macros,DESCRIPTION,missing)

	#@cd $(XORG_UTIL_MACROS_DIR) && make install
	#@$(call install_lib, xorg-util-macros, 0, 0, 0644, libxcb-image)

	@$(call install_finish, xorg-util-macros)

	@$(call touch)


# vim: syntax=make
