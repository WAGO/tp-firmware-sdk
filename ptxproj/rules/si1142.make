# -*-makefile-*-
#
# Copyright (C) 2019 by Christian Hohnstaedt <christian.hohnstaedt@wago.com>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_SI1142) += si1142

#
# Paths and names
#
SI1142_VERSION	:= 0.0.1
SI1142_MD5		:=
SI1142		:= si1142-$(SI1142_VERSION)
SI1142_URL		:= file://local_src/si1142
SI1142_DIR		:= $(BUILDDIR)/$(SI1142)
SI1142_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/si1142.extract:
#	@$(call targetinfo)
#	@$(call clean, $(SI1142_DIR))
#	@$(call extract, SI1142)
#	@$(call patchin, SI1142)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#SI1142_PATH	:= PATH=$(CROSS_PATH)
SI1142_CONF_TOOL	:= NO
SI1142_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/si1142.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, SI1142)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/si1142.compile:
#	@$(call targetinfo)
#	@$(call world/compile, SI1142)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/si1142.install:
#	@$(call targetinfo)
#	@$(call world/install, SI1142)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/si1142.targetinstall:
	@$(call targetinfo)

	@$(call install_init, si1142)
	@$(call install_fixup, si1142,PRIORITY,optional)
	@$(call install_fixup, si1142,SECTION,base)
	@$(call install_fixup, si1142,AUTHOR,"Christian Hohnstaedt <christian.hohnstaedt@wago.com>")
	@$(call install_fixup, si1142,DESCRIPTION,missing)

	@$(call install_copy, si1142, 0, 0, 0755, $(SI1142_DIR)/si1142, /usr/bin/si1142)
	@$(call install_copy, si1142, 0, 0, 0755, $(SI1142_DIR)/si1142_client, /usr/bin/si1142_client)

	@$(call install_finish, si1142)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/si1142.clean:
	@$(call targetinfo)
	@-cd $(SI1142_DIR) && \
		$(SI1142_ENV) $(SI1142_PATH) $(MAKE) clean
	@$(call clean_pkg, SI1142)

# vim: syntax=make
