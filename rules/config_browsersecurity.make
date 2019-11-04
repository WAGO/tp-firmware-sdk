# -*-makefile-*-
#
# Copyright (C) 2015 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CONFIG_BROWSERSECURITY) += config_browsersecurity

#
# Paths and names
#
CONFIG_BROWSERSECURITY_VERSION	:= 1
CONFIG_BROWSERSECURITY_MD5		:=
CONFIG_BROWSERSECURITY		:= config_browsersecurity
CONFIG_BROWSERSECURITY_URL		:= file://local_src/$(CONFIG_BROWSERSECURITY)
CONFIG_BROWSERSECURITY_DIR		:= $(BUILDDIR)/$(CONFIG_BROWSERSECURITY)
CONFIG_BROWSERSECURITY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_browsersecurity.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_BROWSERSECURITY_DIR))
#	@$(call extract, CONFIG_BROWSERSECURITY)
#	@$(call patchin, CONFIG_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_BROWSERSECURITY_PATH	:= PATH=$(CROSS_PATH)
CONFIG_BROWSERSECURITY_CONF_TOOL	:= NO
CONFIG_BROWSERSECURITY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_browsersecurity.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_browsersecurity.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_browsersecurity.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_browsersecurity.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_browsersecurity)
	@$(call install_fixup, config_browsersecurity,PRIORITY,optional)
	@$(call install_fixup, config_browsersecurity,SECTION,base)
	@$(call install_fixup, config_browsersecurity,AUTHOR,"<elrest>")
	@$(call install_fixup, config_browsersecurity,DESCRIPTION,missing)

	@$(call install_copy, config_browsersecurity, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_browsersecurity, 0, 0, 0755, $(CONFIG_BROWSERSECURITY_DIR)/config_browsersecurity, /etc/config-tools/config_browsersecurity)

	@$(call install_finish, config_browsersecurity)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_browsersecurity.clean:
	@$(call targetinfo)
	@-cd $(CONFIG_BROWSERSECURITY_DIR) && \
		$(CONFIG_BROWSERSECURITY_ENV) $(CONFIG_BROWSERSECURITY_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_BROWSERSECURITY)

# vim: syntax=make
