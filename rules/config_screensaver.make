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
PACKAGES-$(PTXCONF_CONFIG_SCREENSAVER) += config_screensaver

#
# Paths and names
#
CONFIG_SCREENSAVER_VERSION	:= 0.0.1
CONFIG_SCREENSAVER_MD5		:=
CONFIG_SCREENSAVER		:= config_screensaver
CONFIG_SCREENSAVER_URL		:= file://local_src/$(CONFIG_SCREENSAVER)
CONFIG_SCREENSAVER_DIR		:= $(BUILDDIR)/$(CONFIG_SCREENSAVER)
CONFIG_SCREENSAVER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_screensaver.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_SCREENSAVER_DIR))
#	@$(call extract, CONFIG_SCREENSAVER)
#	@$(call patchin, CONFIG_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_SCREENSAVER_PATH	:= PATH=$(CROSS_PATH)
CONFIG_SCREENSAVER_CONF_TOOL	:= NO
CONFIG_SCREENSAVER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_screensaver.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_screensaver.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_screensaver.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_screensaver.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_screensaver)
	@$(call install_fixup, config_screensaver,PRIORITY,optional)
	@$(call install_fixup, config_screensaver,SECTION,base)
	@$(call install_fixup, config_screensaver,AUTHOR,"<elrest>")
	@$(call install_fixup, config_screensaver,DESCRIPTION,missing)

	@$(call install_copy, config_screensaver, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_screensaver, 0, 0, 0755, $(CONFIG_SCREENSAVER_DIR)/config_screensaver, /etc/config-tools/config_screensaver)

	@$(call install_finish, config_screensaver)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_screensaver.clean:
	@$(call targetinfo)
	cd $(CONFIG_SCREENSAVER_DIR) && rm -f *.o $(CONFIG_SCREENSAVER)	
	@-cd $(CONFIG_SCREENSAVER_DIR) && \
		$(CONFIG_SCREENSAVER_MAKE_ENV) $(CONFIG_SCREENSAVER_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_SCREENSAVER)

# vim: syntax=make
