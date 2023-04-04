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
PACKAGES-$(PTXCONF_CONFIG_ERUNTIME) += config_eruntime

#
# Paths and names
#
CONFIG_ERUNTIME_VERSION	:= 0.0.1
CONFIG_ERUNTIME_MD5		:=
CONFIG_ERUNTIME		    := config_eruntime
CONFIG_ERUNTIME_URL		:= file://local_src/$(CONFIG_ERUNTIME)
CONFIG_ERUNTIME_DIR		:= $(BUILDDIR)/$(CONFIG_ERUNTIME)
CONFIG_ERUNTIME_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_eruntime.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_ERUNTIME_DIR))
#	@$(call extract, CONFIG_ERUNTIME)
#	@$(call patchin, CONFIG_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_ERUNTIME_PATH	:= PATH=$(CROSS_PATH)
CONFIG_ERUNTIME_CONF_TOOL	:= NO
CONFIG_ERUNTIME_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_eruntime.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_eruntime.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_eruntime.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_eruntime.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_eruntime)
	@$(call install_fixup, config_eruntime,PRIORITY,optional)
	@$(call install_fixup, config_eruntime,SECTION,base)
	@$(call install_fixup, config_eruntime,AUTHOR,"<elrest>")
	@$(call install_fixup, config_eruntime,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_eruntime, 0, 0, 0750, $(CONFIG_ERUNTIME_DIR)/config_eruntime, /etc/config-tools/config_eruntime)

	@$(call install_finish, config_eruntime)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_eruntime.clean:
	@$(call targetinfo)
	-cd $(CONFIG_ERUNTIME_DIR) && rm -f *.o $(CONFIG_ERUNTIME)	
	@-cd $(CONFIG_ERUNTIME_DIR) && \
		$(CONFIG_ERUNTIME_MAKE_ENV) $(CONFIG_ERUNTIME_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_ERUNTIME)

# vim: syntax=make
