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
PACKAGES-$(PTXCONF_XCB_UTIL_RENDERUTIL) += xcb-util-renderutil

#
# Paths and names
#
XCB_UTIL_RENDERUTIL_VERSION	:= 0.3.9
XCB_UTIL_RENDERUTIL_MD5		:= 468b119c94da910e1291f3ffab91019a
XCB_UTIL_RENDERUTIL			:= xcb-util-renderutil-$(XCB_UTIL_RENDERUTIL_VERSION)
XCB_UTIL_RENDERUTIL_SUFFIX	:= tar.bz2
XCB_UTIL_RENDERUTIL_URL		:= http://xcb.freedesktop.org/dist/$(XCB_UTIL_RENDERUTIL).$(XCB_UTIL_RENDERUTIL_SUFFIX)
XCB_UTIL_RENDERUTIL_SOURCE	:= $(SRCDIR)/$(XCB_UTIL_RENDERUTIL).$(XCB_UTIL_RENDERUTIL_SUFFIX)
XCB_UTIL_RENDERUTIL_DIR		:= $(BUILDDIR)/$(XCB_UTIL_RENDERUTIL)
XCB_UTIL_RENDERUTIL_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XCB_UTIL_RENDERUTIL_PATH		:= PATH=$(CROSS_PATH)
XCB_UTIL_RENDERUTIL_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XCB_UTIL_RENDERUTIL_CONF_TOOL	:= autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xcb-util-renderutil.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xcb-util-renderutil)
	@$(call install_fixup, xcb-util-renderutil,PRIORITY,optional)
	@$(call install_fixup, xcb-util-renderutil,SECTION,base)
	@$(call install_fixup, xcb-util-renderutil,AUTHOR,"Erwin Rol <erwin@erwinrol.com>")
	@$(call install_fixup, xcb-util-renderutil,DESCRIPTION,missing)

	@$(call install_lib, xcb-util-renderutil, 0, 0, 0644, libxcb-render-util)

	@$(call install_finish, xcb-util-renderutil)

	@$(call touch)


# vim: syntax=make
