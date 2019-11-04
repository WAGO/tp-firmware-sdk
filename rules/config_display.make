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
PACKAGES-$(PTXCONF_CONFIG_DISPLAY) += config_display

#
# Paths and names
#
CONFIG_DISPLAY_VERSION	:= 1
CONFIG_DISPLAY_MD5		:=
CONFIG_DISPLAY		:= config_display
CONFIG_DISPLAY_URL		:= file://local_src/$(CONFIG_DISPLAY)
CONFIG_DISPLAY_DIR		:= $(BUILDDIR)/$(CONFIG_DISPLAY)
CONFIG_DISPLAY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_display.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_DISPLAY_DIR))
#	@$(call extract, CONFIG_DISPLAY)
#	@$(call patchin, CONFIG_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_DISPLAY_PATH	:= PATH=$(CROSS_PATH)
CONFIG_DISPLAY_CONF_TOOL	:= NO
CONFIG_DISPLAY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_display.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_display.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_display.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_display.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_display)
	@$(call install_fixup, config_display,PRIORITY,optional)
	@$(call install_fixup, config_display,SECTION,base)
	@$(call install_fixup, config_display,AUTHOR,"<elrest>")
	@$(call install_fixup, config_display,DESCRIPTION,missing)

	@$(call install_copy, config_display, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_display, 0, 0, 0755, $(CONFIG_DISPLAY_DIR)/config_display, /etc/config-tools/config_display)

	@$(call install_finish, config_display)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_display.clean:
	@$(call targetinfo)
	cd $(CONFIG_DISPLAY_DIR) && rm -f *.o $(CONFIG_DISPLAY)
	@-cd $(CONFIG_DISPLAY_DIR) && \
		$(CONFIG_DISPLAY_MAKE_ENV) $(CONFIG_DISPLAY_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_DISPLAY)

# vim: syntax=make
