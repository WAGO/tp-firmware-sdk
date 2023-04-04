# -*-makefile-*-
#
# Copyright (C) 2014 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CONFIG_CONFIGURED) += config_configured

#
# Paths and names
#
CONFIG_CONFIGURED_VERSION	:= 1
CONFIG_CONFIGURED_MD5		:=
CONFIG_CONFIGURED		:= config_configured
CONFIG_CONFIGURED_URL		:= file://local_src/$(CONFIG_CONFIGURED)
CONFIG_CONFIGURED_DIR		:= $(BUILDDIR)/$(CONFIG_CONFIGURED)
CONFIG_CONFIGURED_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_configured.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_CONFIGURED_DIR))
#	@$(call extract, CONFIG_CONFIGURED)
#	@$(call patchin, CONFIG_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_CONFIGURED_PATH	:= PATH=$(CROSS_PATH)
CONFIG_CONFIGURED_CONF_TOOL	:= NO
CONFIG_CONFIGURED_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_configured.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_configured.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_configured.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_configured.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_configured)
	@$(call install_fixup, config_configured,PRIORITY,optional)
	@$(call install_fixup, config_configured,SECTION,base)
	@$(call install_fixup, config_configured,AUTHOR,"<elrest>")
	@$(call install_fixup, config_configured,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_configured, 0, 0, 0750, $(CONFIG_CONFIGURED_DIR)/config_configured, /etc/config-tools/config_configured)

	@$(call install_finish, config_configured)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_configured.clean:
	@$(call targetinfo)
	-cd $(CONFIG_CONFIGURED_DIR) && rm -f *.o $(CONFIG_CONFIGURED)	
	@-cd $(CONFIG_CONFIGURED_DIR) && \
		$(CONFIG_CONFIGURED_MAKE_ENV) $(CONFIG_CONFIGURED_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_CONFIGURED)

# vim: syntax=make
