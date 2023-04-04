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
PACKAGES-$(PTXCONF_CONFIG_FONTS) += config_fonts

#
# Paths and names
#
CONFIG_FONTS_VERSION	:= 1
CONFIG_FONTS_MD5		:=
CONFIG_FONTS		:= config_fonts
CONFIG_FONTS_URL		:= file://local_src/$(CONFIG_FONTS)
CONFIG_FONTS_DIR		:= $(BUILDDIR)/$(CONFIG_FONTS)
CONFIG_FONTS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_fonts.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_FONTS_DIR))
#	@$(call extract, CONFIG_FONTS)
#	@$(call patchin, CONFIG_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_FONTS_PATH	:= PATH=$(CROSS_PATH)
CONFIG_FONTS_CONF_TOOL	:= NO
CONFIG_FONTS_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_fonts.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_fonts.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_fonts.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_fonts.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_fonts)
	@$(call install_fixup, config_fonts,PRIORITY,optional)
	@$(call install_fixup, config_fonts,SECTION,base)
	@$(call install_fixup, config_fonts,AUTHOR,"<elrest>")
	@$(call install_fixup, config_fonts,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_fonts, 0, 0, 0750, $(CONFIG_FONTS_DIR)/config_fonts, /etc/config-tools/config_fonts)

	@$(call install_finish, config_fonts)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_fonts.clean:
	@$(call targetinfo)
	@-cd $(CONFIG_FONTS_DIR) && \
		$(CONFIG_FONTS_ENV) $(CONFIG_FONTS_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_FONTS)

# vim: syntax=make
