# -*-makefile-*-
#
# Copyright (C) 2006 by Sascha Hauer
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XORG_DRIVER_INPUT_TSLIB) += xorg-driver-input-tslib

#
# Paths and names
#
XORG_DRIVER_INPUT_TSLIB_VERSION	:= 0.0.7
XORG_DRIVER_INPUT_TSLIB_MD5	:= dea5f1f7713f69fb37353320fd4e67a8
XORG_DRIVER_INPUT_TSLIB		:= xf86-input-tslib-$(XORG_DRIVER_INPUT_TSLIB_VERSION)
XORG_DRIVER_INPUT_TSLIB_SUFFIX	:= tar.bz2
XORG_DRIVER_INPUT_TSLIB_URL	:= http://pengutronix.de/software/xf86-input-tslib/download/$(XORG_DRIVER_INPUT_TSLIB).$(XORG_DRIVER_INPUT_TSLIB_SUFFIX)
XORG_DRIVER_INPUT_TSLIB_SOURCE	:= $(SRCDIR)/$(XORG_DRIVER_INPUT_TSLIB).$(XORG_DRIVER_INPUT_TSLIB_SUFFIX)
XORG_DRIVER_INPUT_TSLIB_DIR	:= $(BUILDDIR)/$(XORG_DRIVER_INPUT_TSLIB)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XORG_DRIVER_INPUT_TSLIB_PATH	:= PATH=$(CROSS_PATH)
XORG_DRIVER_INPUT_TSLIB_ENV 	:= $(CROSS_ENV)

#
# autoconf
#
XORG_DRIVER_INPUT_TSLIB_AUTOCONF := $(CROSS_AUTOCONF_USR)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xorg-driver-input-tslib.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xorg-driver-input-tslib)
	@$(call install_fixup, xorg-driver-input-tslib,PRIORITY,optional)
	@$(call install_fixup, xorg-driver-input-tslib,SECTION,base)
	@$(call install_fixup, xorg-driver-input-tslib,AUTHOR,"Erwin Rol <ero@pengutronix.de>")
	@$(call install_fixup, xorg-driver-input-tslib,DESCRIPTION,missing)

	@$(call install_copy, xorg-driver-input-tslib, 0, 0, 0755, -, \
		/usr/lib/xorg/modules/input/tslib_drv.so)

	@$(call install_finish, xorg-driver-input-tslib)

	@$(call touch)

# vim: syntax=make
