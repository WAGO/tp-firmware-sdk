# -*-makefile-*-
#
# Copyright (C) 2025 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_ICEWM) += icewm

#
# Paths and names
#
ICEWM_VERSION	:= 3.8.1
ICEWM_MD5		:= 1d94860a064a33331937b1cd8f1ae480
ICEWM			:= icewm-$(ICEWM_VERSION)
ICEWM_SUFFIX	:= tar.gz
ICEWM_URL		:= https://icewm.org/files/source/$(ICEWM).$(ICEWM_SUFFIX)
ICEWM_SOURCE	:= $(SRCDIR)/$(ICEWM).$(ICEWM_SUFFIX)
ICEWM_DIR		:= $(BUILDDIR)/$(ICEWM)
ICEWM_LICENSE	:= GPL-2.0-or-later

# ----------------------------------------------------------------------------
# Get
# ----------------------------------------------------------------------------

$(ICEWM_SOURCE):
	@$(call targetinfo)
	@$(call get, ICEWM)
	@$(call touch)

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------
$(STATEDIR)/ICEWM.extract:
	@$(call targetinfo)
	@$(call clean, $(ICEWM_DIR))
	@$(call extract, ICEWM)
	@$(call patchin, ICEWM)
	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------
ICEWM_CONF_ENV	:= $(CROSS_ENV)

# cmake
ICEWM_CONF_TOOL	:= cmake

ICEWM_CONF_OPT	:= $(CROSS_CMAKE_USR) \
	-DCONFIG_IMLIB2=off \
	-DCONFIG_GDK_PIXBUF_XLIB=on \
	-DCONFIG_LIBPNG=on \
	-DCONFIG_XPM=on \
	-DCONFIG_LIBJPEG=on \
	-DCFGDIR="/etc/specific"

#	--debug-output

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/icewm.targetinstall:
	@$(call targetinfo)

	@$(call install_init, icewm)
	@$(call install_fixup, icewm,PRIORITY,optional)
	@$(call install_fixup, icewm,SECTION,base)
	@$(call install_fixup, icewm,AUTHOR, "Bert Gijsbers")
	@$(call install_fixup, icewm,DESCRIPTION,missing)

	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icehelp)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icesh)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewm)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewmbg)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewmhint)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewm-session)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewm-menu-fdo)
	@$(call install_copy, icewm, 0, 0, 0755, -, /usr/bin/icewm-menu-xrandr)
	@$(call install_tree, icewm, 0, 0, -, /usr/share/locale)

#	use only default theme installed by elrest-custom-install
#	@$(call install_tree, icewm, 0, 0, -, /usr/share/icewm)

	@$(call install_finish, icewm)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/ICEWM.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, ICEWM)

# vim: syntax=make
