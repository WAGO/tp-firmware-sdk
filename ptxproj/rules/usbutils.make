# -*-makefile-*-
#
# Copyright (C) 2007 by Sascha Hauer
#               2009, 2010 by Marc Kleine-Budde <mkl@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_USBUTILS) += usbutils

#
# Paths and names
#
USBUTILS_VERSION	:= 007
USBUTILS_MD5		:= c9df5107ae9d26b10a1736a261250139
USBUTILS		:= usbutils-$(USBUTILS_VERSION)
USBUTILS_SUFFIX		:= tar.xz
USBUTILS_URL		:= $(call ptx/mirror, KERNEL, utils/usb/usbutils/$(USBUTILS).$(USBUTILS_SUFFIX))
USBUTILS_SOURCE		:= $(SRCDIR)/$(USBUTILS).$(USBUTILS_SUFFIX)
USBUTILS_DIR		:= $(BUILDDIR)/$(USBUTILS)
USBUTILS_LICENSE	:= GPL-2.0-only
USBUTILS_DEVPKG		:= NO

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

USBUTILS_PATH	:= PATH=$(CROSS_PATH)
USBUTILS_ENV 	:= $(CROSS_ENV)

#
# autoconf
#
USBUTILS_AUTOCONF := \
	$(CROSS_AUTOCONF_USR) \
	$(GLOBAL_LARGE_FILE_OPTION) \
	--disable-zlib \
	--enable-usbids

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/usbutils.targetinstall:
	@$(call targetinfo)

	@$(call install_init, usbutils)
	@$(call install_fixup, usbutils,PRIORITY,optional)
	@$(call install_fixup, usbutils,SECTION,base)
	@$(call install_fixup, usbutils,AUTHOR,"Robert Schwebel <r.schwebel@pengutronix.de>")
	@$(call install_fixup, usbutils,DESCRIPTION,missing)

ifdef PTXCONF_USBUTILS_LSUSB
	@$(call install_copy, usbutils, 0, 0, 0755, -, /usr/bin/lsusb)
endif
ifdef PTXCONF_USBUTILS_USBHIDDUMP
	@$(call install_copy, usbutils, 0, 0, 0755, -, /usr/bin/usbhid-dump)
endif
ifdef PTXCONF_USBUTILS_USBDEVICES
	@$(call install_copy, usbutils, 0, 0, 0755, -, /usr/bin/usb-devices)
endif
#   PTXdist-Update 2024.12.0: usb.ids is now provided by hwdata package
#     We cannot update usbutils to new version, since we are stuck
#     to udev 182 and new usbutils require a later version.
#	@$(call install_copy, usbutils, 0, 0, 0644, -, /usr/share/usb.ids,n)

#   PTXdist-Update 2024.12.0: install license file
#     Since usb.ids are no longer provided by the package, nothing
#     is provided by the package for some images and the package is omitted.
#     We install at least the license file to mark that the package is part
#     of our image.
	@$(call install_copy, usbutils, 0, 0, 0644, $(USBUTILS_DIR)/COPYING, /usr/share/licenses/oss/license.usbutils_$(USBUTILS_VERSION).txt)

	@$(call install_finish, usbutils)

	@$(call touch)

# vim: syntax=make
