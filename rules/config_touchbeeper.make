# -*-makefile-*-
#
# Copyright (C) 2013 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CONFIG_TOUCHBEEPER) += config_touchbeeper

#
# Paths and names
#
CONFIG_TOUCHBEEPER_VERSION	:= 0.0.1
CONFIG_TOUCHBEEPER_MD5		:=
CONFIG_TOUCHBEEPER		:= config_touchbeeper
CONFIG_TOUCHBEEPER_URL		:= file://local_src/$(CONFIG_TOUCHBEEPER)
CONFIG_TOUCHBEEPER_DIR		:= $(BUILDDIR)/$(CONFIG_TOUCHBEEPER)
CONFIG_TOUCHBEEPER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_touchbeeper.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_TOUCHBEEPER_DIR))
#	@$(call extract, CONFIG_TOUCHBEEPER)
#	@$(call patchin, CONFIG_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_TOUCHBEEPER_PATH	:= PATH=$(CROSS_PATH)
CONFIG_TOUCHBEEPER_CONF_TOOL	:= NO
CONFIG_TOUCHBEEPER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_touchbeeper.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_touchbeeper.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_touchbeeper.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_touchbeeper.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_touchbeeper)
	@$(call install_fixup, config_touchbeeper,PRIORITY,optional)
	@$(call install_fixup, config_touchbeeper,SECTION,base)
	@$(call install_fixup, config_touchbeeper,AUTHOR,"<elrest>")
	@$(call install_fixup, config_touchbeeper,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_touchbeeper, 0, 0, 0755, $(CONFIG_TOUCHBEEPER_DIR)/config_touchbeeper, /etc/config-tools/config_touchbeeper)

	@$(call install_finish, config_touchbeeper)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_touchbeeper.clean:
	@$(call targetinfo)
	-cd $(CONFIG_TOUCHBEEPER_DIR) && rm -f *.o $(CONFIG_TOUCHBEEPER)	
	@-cd $(CONFIG_TOUCHBEEPER_DIR) && \
		$(CONFIG_TOUCHBEEPER_MAKE_ENV) $(CONFIG_TOUCHBEEPER_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_TOUCHBEEPER)

# vim: syntax=make
