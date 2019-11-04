# -*-makefile-*-
#
# Copyright (C) 2010 by Erwin Rol <erwin@erwinrol.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XCB_UTIL_WM) += xcb-util-wm

#
# Paths and names
#
XCB_UTIL_WM_VERSION	:= 0.3.9
XCB_UTIL_WM_MD5		:= c612aae7ffe2e70fe743efab9c5548e9
XCB_UTIL_WM			:= xcb-util-wm-$(XCB_UTIL_WM_VERSION)
XCB_UTIL_WM_SUFFIX	:= tar.bz2
XCB_UTIL_WM_URL		:= http://xcb.freedesktop.org/dist/$(XCB_UTIL_WM).$(XCB_UTIL_WM_SUFFIX)
XCB_UTIL_WM_SOURCE	:= $(SRCDIR)/$(XCB_UTIL_WM).$(XCB_UTIL_WM_SUFFIX)
XCB_UTIL_WM_DIR		:= $(BUILDDIR)/$(XCB_UTIL_WM)
XCB_UTIL_WM_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XCB_UTIL_WM_PATH		:= PATH=$(CROSS_PATH)
XCB_UTIL_WM_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XCB_UTIL_WM_CONF_TOOL	:= autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xcb-util-wm.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xcb-util-wm)
	@$(call install_fixup, xcb-util-wm,PRIORITY,optional)
	@$(call install_fixup, xcb-util-wm,SECTION,base)
	@$(call install_fixup, xcb-util-wm,AUTHOR,"Erwin Rol <erwin@erwinrol.com>")
	@$(call install_fixup, xcb-util-wm,DESCRIPTION,missing)

	@$(call install_lib, xcb-util-wm, 0, 0, 0644, libxcb-ewmh)
	@$(call install_lib, xcb-util-wm, 0, 0, 0644, libxcb-icccm)

	@$(call install_finish, xcb-util-wm)

	@$(call touch)


# vim: syntax=make
