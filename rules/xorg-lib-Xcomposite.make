# -*-makefile-*-
#
# Copyright (C) 2006 by Erwin Rol
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_LIB_XCOMPOSITE) += xorg-lib-xcomposite

#
# Paths and names
#
XORG_LIB_XCOMPOSITE_VERSION	:= 0.4.5
XORG_LIB_XCOMPOSITE_MD5		:= 3fa0841ea89024719b20cd702a9b54e0
XORG_LIB_XCOMPOSITE		:= libXcomposite-$(XORG_LIB_XCOMPOSITE_VERSION)
XORG_LIB_XCOMPOSITE_SUFFIX	:= tar.bz2
XORG_LIB_XCOMPOSITE_URL		:= $(call ptx/mirror, XORG, individual/lib/$(XORG_LIB_XCOMPOSITE).$(XORG_LIB_XCOMPOSITE_SUFFIX))
XORG_LIB_XCOMPOSITE_SOURCE	:= $(SRCDIR)/$(XORG_LIB_XCOMPOSITE).$(XORG_LIB_XCOMPOSITE_SUFFIX)
XORG_LIB_XCOMPOSITE_DIR		:= $(BUILDDIR)/$(XORG_LIB_XCOMPOSITE)
XORG_LIB_XCOMPOSITE_LICENSE	:= MIT

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
XORG_LIB_XCOMPOSITE_CONF_TOOL := autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-lib-xcomposite.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xorg-lib-xcomposite)
	@$(call install_fixup, xorg-lib-xcomposite,PRIORITY,optional)
	@$(call install_fixup, xorg-lib-xcomposite,SECTION,base)
	@$(call install_fixup, xorg-lib-xcomposite,AUTHOR,"Erwin Rol <ero@pengutronix.de>")
	@$(call install_fixup, xorg-lib-xcomposite,DESCRIPTION,missing)

	@$(call install_lib, xorg-lib-xcomposite, 0, 0, 0644, libXcomposite)

	@$(call install_finish, xorg-lib-xcomposite)

	@$(call touch)

# vim: syntax=make
