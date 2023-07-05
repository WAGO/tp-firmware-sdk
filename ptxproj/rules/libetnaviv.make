# -*-makefile-*-
#
# Copyright (C) 2017 by <RG>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_LIBETNAVIV) += libetnaviv

#
# Paths and names
#
LIBETNAVIV_VERSION	:= 1.0
LIBETNAVIV_MD5		:= 3bbccbb1292944f5f5fc4afe9b573315
LIBETNAVIV		:= libetnaviv-$(LIBETNAVIV_VERSION)
LIBETNAVIV_SUFFIX	:= tar.bz2
LIBETNAVIV_URL		:= https://svsv01003/svn/repo14/$(LIBETNAVIV).$(LIBETNAVIV_SUFFIX)
LIBETNAVIV_SOURCE	:= $(SRCDIR)/$(LIBETNAVIV).$(LIBETNAVIV_SUFFIX)
LIBETNAVIV_DIR		:= $(BUILDDIR)/$(LIBETNAVIV)
LIBETNAVIV_BUILD_OOT	:= YES
LIBETNAVIV_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#LIBETNAVIV_CONF_ENV	:= $(CROSS_ENV)

#
# autoconf
#
LIBETNAVIV_CONF_TOOL	:= autoconf
LIBETNAVIV_CONF_OPT	:= $(CROSS_AUTOCONF_USR) \
	CFLAGS="-I$(PTXCONF_SYSROOT_TARGET)/usr/include  -I$(PTXCONF_SYSROOT_TARGET)/usr/include/galcore_headers/include_imx6_v4_6_9"

#$(STATEDIR)/libetnaviv.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, LIBETNAVIV)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/libetnaviv.compile:
#	@$(call targetinfo)
#	@$(call world/compile, LIBETNAVIV)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/libetnaviv.install:
#	@$(call targetinfo)
#	@$(call world/install, LIBETNAVIV)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/libetnaviv.targetinstall:
	@$(call targetinfo)

	@$(call install_init, libetnaviv)
	@$(call install_fixup, libetnaviv, PRIORITY, optional)
	@$(call install_fixup, libetnaviv, SECTION, base)
	@$(call install_fixup, libetnaviv, AUTHOR, "<RG>")
	@$(call install_fixup, libetnaviv, DESCRIPTION, missing)

	@$(call install_lib, libetnaviv, 0, 0, 0644, libetnaviv)

	@$(call install_finish, libetnaviv)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

#$(STATEDIR)/libetnaviv.clean:
#	@$(call targetinfo)
#	@$(call clean_pkg, LIBETNAVIV)

# vim: syntax=make