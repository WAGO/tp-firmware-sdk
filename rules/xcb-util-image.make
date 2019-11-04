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
PACKAGES-$(PTXCONF_XCB_UTIL_IMAGE) += xcb-util-image

#
# Paths and names
#
XCB_UTIL_IMAGE_VERSION	:= 0.3.9
XCB_UTIL_IMAGE_MD5		:= fabb80b36490b00fc91289e2c7f66770
XCB_UTIL_IMAGE			:= xcb-util-image-$(XCB_UTIL_IMAGE_VERSION)
XCB_UTIL_IMAGE_SUFFIX	:= tar.bz2
XCB_UTIL_IMAGE_URL		:= http://xcb.freedesktop.org/dist/$(XCB_UTIL_IMAGE).$(XCB_UTIL_IMAGE_SUFFIX)
XCB_UTIL_IMAGE_SOURCE	:= $(SRCDIR)/$(XCB_UTIL_IMAGE).$(XCB_UTIL_IMAGE_SUFFIX)
XCB_UTIL_IMAGE_DIR		:= $(BUILDDIR)/$(XCB_UTIL_IMAGE)
XCB_UTIL_IMAGE_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

XCB_UTIL_IMAGE_PATH		:= PATH=$(CROSS_PATH)
XCB_UTIL_IMAGE_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
XCB_UTIL_IMAGE_CONF_TOOL	:= autoconf

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xcb-util-image.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xcb-util-image)
	@$(call install_fixup, xcb-util-image,PRIORITY,optional)
	@$(call install_fixup, xcb-util-image,SECTION,base)
	@$(call install_fixup, xcb-util-image,AUTHOR,"Erwin Rol <erwin@erwinrol.com>")
	@$(call install_fixup, xcb-util-image,DESCRIPTION,missing)

	@$(call install_lib, xcb-util-image, 0, 0, 0644, libxcb-image)

	@$(call install_finish, xcb-util-image)

	@$(call touch)


# vim: syntax=make
