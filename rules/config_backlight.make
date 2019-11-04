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
PACKAGES-$(PTXCONF_CONFIG_BACKLIGHT) += config_backlight

#
# Paths and names
#
CONFIG_BACKLIGHT_VERSION	:= 0.0.1
CONFIG_BACKLIGHT_MD5		:=
CONFIG_BACKLIGHT		:= config_backlight
CONFIG_BACKLIGHT_URL		:= file://local_src/$(CONFIG_BACKLIGHT)
CONFIG_BACKLIGHT_DIR		:= $(BUILDDIR)/$(CONFIG_BACKLIGHT)
CONFIG_BACKLIGHT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_backlight.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_BACKLIGHT_DIR))
#	@$(call extract, CONFIG_BACKLIGHT)
#	@$(call patchin, CONFIG_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_BACKLIGHT_PATH	:= PATH=$(CROSS_PATH)
CONFIG_BACKLIGHT_CONF_TOOL	:= NO
CONFIG_BACKLIGHT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_backlight.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_backlight.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_backlight.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_backlight.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_backlight)
	@$(call install_fixup, config_backlight,PRIORITY,optional)
	@$(call install_fixup, config_backlight,SECTION,base)
	@$(call install_fixup, config_backlight,AUTHOR,"<elrest>")
	@$(call install_fixup, config_backlight,DESCRIPTION,missing)

	@$(call install_copy, config_backlight, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_backlight, 0, 0, 0755, $(CONFIG_BACKLIGHT_DIR)/config_backlight, /etc/config-tools/config_backlight)

	@$(call install_finish, config_backlight)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_backlight.clean:
	@$(call targetinfo)
	cd $(CONFIG_BACKLIGHT_DIR) && rm -f *.o $(CONFIG_BACKLIGHT)	
	@-cd $(CONFIG_BACKLIGHT_DIR) && \
		$(CONFIG_BACKLIGHT_MAKE_ENV) $(CONFIG_BACKLIGHT_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_BACKLIGHT)

# vim: syntax=make
