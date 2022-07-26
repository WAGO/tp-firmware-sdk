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
PACKAGES-$(PTXCONF_CONFIG_PLCSELECT) += config_plcselect

#
# Paths and names
#
CONFIG_PLCSELECT_VERSION	:= 1
CONFIG_PLCSELECT_MD5		:=
CONFIG_PLCSELECT		:= config_plcselect
CONFIG_PLCSELECT_URL		:= file://local_src/$(CONFIG_PLCSELECT)
CONFIG_PLCSELECT_DIR		:= $(BUILDDIR)/$(CONFIG_PLCSELECT)
CONFIG_PLCSELECT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_plcselect.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_PLCSELECT_DIR))
#	@$(call extract, CONFIG_PLCSELECT)
#	@$(call patchin, CONFIG_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_PLCSELECT_PATH	:= PATH=$(CROSS_PATH)
CONFIG_PLCSELECT_CONF_TOOL	:= NO
CONFIG_PLCSELECT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_plcselect.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_plcselect.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_plcselect.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_plcselect.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_plcselect)
	@$(call install_fixup, config_plcselect,PRIORITY,optional)
	@$(call install_fixup, config_plcselect,SECTION,base)
	@$(call install_fixup, config_plcselect,AUTHOR,"<elrest>")
	@$(call install_fixup, config_plcselect,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_plcselect, 0, 0, 0755, $(CONFIG_PLCSELECT_DIR)/config_plcselect, /etc/config-tools/config_plcselect)

	@$(call install_finish, config_plcselect)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_plcselect.clean:
	@$(call targetinfo)
	-cd $(CONFIG_PLCSELECT_DIR) && rm -f *.o $(CONFIG_PLCSELECT)
	@-cd $(CONFIG_PLCSELECT_DIR) && \
		$(CONFIG_PLCSELECT_MAKE_ENV) $(CONFIG_PLCSELECT_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_PLCSELECT)

# vim: syntax=make
