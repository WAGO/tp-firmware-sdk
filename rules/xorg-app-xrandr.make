# -*-makefile-*-
#
# Copyright (C) 2009 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_APP_XRANDR) += xorg-app-xrandr

#
# Paths and names
#
XORG_APP_XRANDR_VERSION	:= 1.5.1
XORG_APP_XRANDR_MD5	:= fe40f7a4fd39dd3a02248d3e0b1972e4
XORG_APP_XRANDR		:= xrandr-$(XORG_APP_XRANDR_VERSION)
XORG_APP_XRANDR_SUFFIX	:= tar.xz
XORG_APP_XRANDR_URL	:= $(call ptx/mirror, XORG, individual/app/$(XORG_APP_XRANDR).$(XORG_APP_XRANDR_SUFFIX))
XORG_APP_XRANDR_SOURCE	:= $(SRCDIR)/$(XORG_APP_XRANDR).$(XORG_APP_XRANDR_SUFFIX)
XORG_APP_XRANDR_DIR	:= $(BUILDDIR)/$(XORG_APP_XRANDR)
XORG_APP_XRANDR_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# autoconf
#
XORG_APP_XRANDR_CONF_TOOL := autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-app-xrandr.targetinstall:
	@$(call targetinfo)

	@$(call install_init,  xorg-app-xrandr)
	@$(call install_fixup, xorg-app-xrandr,PRIORITY,optional)
	@$(call install_fixup, xorg-app-xrandr,SECTION,base)
	@$(call install_fixup, xorg-app-xrandr,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, xorg-app-xrandr,DESCRIPTION,missing)

	@$(call install_copy, xorg-app-xrandr, 0, 0, 0755, -, \
		/usr/bin/xrandr)
	@$(call install_copy, xorg-app-xrandr, 0, 0, 0755, -, \
		/usr/bin/xkeystone)

	@$(call install_finish, xorg-app-xrandr)

	@$(call touch)

# vim: syntax=make
