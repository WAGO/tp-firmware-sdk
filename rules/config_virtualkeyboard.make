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
PACKAGES-$(PTXCONF_CONFIG_VIRTUALKEYBOARD) += config_virtualkeyboard

#
# Paths and names
#
CONFIG_VIRTUALKEYBOARD_VERSION	:= 1
CONFIG_VIRTUALKEYBOARD_MD5		:=
CONFIG_VIRTUALKEYBOARD		:= config_virtualkeyboard
CONFIG_VIRTUALKEYBOARD_URL		:= file://local_src/$(CONFIG_VIRTUALKEYBOARD)
CONFIG_VIRTUALKEYBOARD_DIR		:= $(BUILDDIR)/$(CONFIG_VIRTUALKEYBOARD)
CONFIG_VIRTUALKEYBOARD_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_virtualkeyboard.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_VIRTUALKEYBOARD_DIR))
#	@$(call extract, CONFIG_VIRTUALKEYBOARD)
#	@$(call patchin, CONFIG_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_VIRTUALKEYBOARD_PATH	:= PATH=$(CROSS_PATH)
CONFIG_VIRTUALKEYBOARD_CONF_TOOL	:= NO
CONFIG_VIRTUALKEYBOARD_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_virtualkeyboard.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_virtualkeyboard.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_virtualkeyboard.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_virtualkeyboard.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_virtualkeyboard)
	@$(call install_fixup, config_virtualkeyboard,PRIORITY,optional)
	@$(call install_fixup, config_virtualkeyboard,SECTION,base)
	@$(call install_fixup, config_virtualkeyboard,AUTHOR,"<elrest>")
	@$(call install_fixup, config_virtualkeyboard,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_virtualkeyboard, 0, 0, 0755, $(CONFIG_VIRTUALKEYBOARD_DIR)/config_virtualkeyboard, /etc/config-tools/config_virtualkeyboard)

	@$(call install_finish, config_virtualkeyboard)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_virtualkeyboard.clean:
	@$(call targetinfo)
	cd $(CONFIG_VIRTUALKEYBOARD_DIR) && rm -f *.o $(CONFIG_VIRTUALKEYBOARD)	
	@-cd $(CONFIG_VIRTUALKEYBOARD_DIR) && \
		$(CONFIG_VIRTUALKEYBOARD_ENV) $(CONFIG_VIRTUALKEYBOARD_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_VIRTUALKEYBOARD)

# vim: syntax=make
