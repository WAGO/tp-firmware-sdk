# -*-makefile-*-
#
# Copyright (C) 2015 by Michael Olbrich <m.olbrich@pengutronix.de>
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBQMI) += libqmi

#
# Paths and names
#
LIBQMI_VERSION	:= 1.34.0
LIBQMI_MD5	:= 677b5d1ab763a7b7285b82d1798ff93d
LIBQMI		:= libqmi-$(LIBQMI_VERSION)
LIBQMI_SUFFIX	:= tar.bz2
LIBQMI_URL	:= https://gitlab.freedesktop.org/mobile-broadband/libqmi/-/archive/$(LIBQMI_VERSION)/$(LIBQMI).$(LIBQMI_SUFFIX)
LIBQMI_SOURCE	:= $(SRCDIR)/$(LIBQMI).$(LIBQMI_SUFFIX)
LIBQMI_DIR	:= $(BUILDDIR)/$(LIBQMI)
LIBQMI_LICENSE	:= GPL-2.0-or-later AND LGPL-2.1-or-later
LIBQMI_LICENSE_FILES := \
	file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
	file://COPYING.LIB;md5=4fbd65380cdd255951079008b364516c
LIBQMI_DEVPKG := NO
#Filename for license text of libqmi-glib (LGPL)
LIBQMI_LICENSE_FILE_LIB :=  COPYING.LIB
#Filename for license text of qmicli and qmi-network (GPL)
#Currently not in use because the binaries are not needed
LIBQMI_LICENSE_FILE := COPYING
#Filenmae for author list
LIBQMI_AUTHORS := AUTHORS

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#
# meson
#
LIBQMI_CONF_TOOL	:= meson
LIBQMI_CONF_OPT		:= \
	$(CROSS_MESON_USR) \
	-Dc_flags="$(CFLAGS) -g -Os" \
	-Dfirmware_update=$(call ptx/truefalse, PTXCONF_LIBQMI_FIRMWARE_UPDATE) \
	-Dcollection=full \
	-Dmbim_qmux=$(call ptx/truefalse, PTXCONF_LIBQMI_MBIM_QMUX) \
	-Dmm_runtime_check=false \
	-Dqrtr=false \
	-Drmnet=false \
	-Dudev=false \
	-Dudevdir=/usr/lib/udev \
	-Dintrospection=false \
	-Dgtk_doc=false \
	-Dman=false \
	-Dbash_completion=false

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libqmi.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libqmi)
	@$(call install_fixup, libqmi,PRIORITY,optional)
	@$(call install_fixup, libqmi,SECTION,base)
	@$(call install_fixup, libqmi,AUTHOR,"Michael Olbrich <m.olbrich@pengutronix.de>")
	@$(call install_fixup, libqmi,DESCRIPTION,missing)

ifdef PTXCONF_LIBQMI_FIRMWARE_UPDATE
	@$(call install_copy, libqmi, 0, 0, 0755, -, /usr/bin/qmi-firmware-update)
endif

	@cat $(LIBQMI_DIR)/$(LIBQMI_AUTHORS) $(LIBQMI_DIR)/$(LIBQMI_LICENSE_FILE_LIB) > $(LIBQMI_DIR)/libqmi-license
	@$(call install_copy, libqmi, 0, 0, 0644, $(LIBQMI_DIR)/libqmi-license, /usr/share/licenses/oss/license.libqmi_$(LIBQMI_VERSION).txt)

	@$(call install_lib, libqmi, 0, 0, 0644, libqmi-glib)

	@$(call install_finish, libqmi)

	@$(call touch)

# vim: syntax=make
