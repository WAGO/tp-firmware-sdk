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
PACKAGES-$(PTXCONF_XCB_UTIL_KEYSYMS) += xcb-util-keysyms

#
# Paths and names
#
XCB_UTIL_KEYSYMS_VERSION	:= 0.3.9
XCB_UTIL_KEYSYMS_MD5		:= 64e4aad2d48cd4a92e2da13b9f35bfd2
XCB_UTIL_KEYSYMS			:= xcb-util-keysyms-$(XCB_UTIL_KEYSYMS_VERSION)
XCB_UTIL_KEYSYMS_SUFFIX	:= tar.bz2
XCB_UTIL_KEYSYMS_URL		:= http://xcb.freedesktop.org/dist/$(XCB_UTIL_KEYSYMS).$(XCB_UTIL_KEYSYMS_SUFFIX)
XCB_UTIL_KEYSYMS_SOURCE	:= $(SRCDIR)/$(XCB_UTIL_KEYSYMS).$(XCB_UTIL_KEYSYMS_SUFFIX)
XCB_UTIL_KEYSYMS_DIR		:= $(BUILDDIR)/$(XCB_UTIL_KEYSYMS)
XCB_UTIL_KEYSYMS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XCB_UTIL_KEYSYMS_PATH		:= PATH=$(CROSS_PATH)
XCB_UTIL_KEYSYMS_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XCB_UTIL_KEYSYMS_CONF_TOOL	:= autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xcb-util-keysyms.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xcb-util-keysyms)
	@$(call install_fixup, xcb-util-keysyms,PRIORITY,optional)
	@$(call install_fixup, xcb-util-keysyms,SECTION,base)
	@$(call install_fixup, xcb-util-keysyms,AUTHOR,"Erwin Rol <erwin@erwinrol.com>")
	@$(call install_fixup, xcb-util-keysyms,DESCRIPTION,missing)

	@$(call install_lib, xcb-util-keysyms, 0, 0, 0644, libxcb-keysyms)

	@$(call install_finish, xcb-util-keysyms)

	@$(call touch)


# vim: syntax=make
