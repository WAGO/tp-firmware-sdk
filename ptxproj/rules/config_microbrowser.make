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
PACKAGES-$(PTXCONF_CONFIG_MICROBROWSER) += config_microbrowser

#
# Paths and names
#
CONFIG_MICROBROWSER_VERSION	:= 0.0.1
CONFIG_MICROBROWSER_MD5		:=
CONFIG_MICROBROWSER		    := config_microbrowser
CONFIG_MICROBROWSER_URL		:= file://local_src/$(CONFIG_MICROBROWSER)
CONFIG_MICROBROWSER_DIR		:= $(BUILDDIR)/$(CONFIG_MICROBROWSER)
CONFIG_MICROBROWSER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_microbrowser.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_MICROBROWSER_DIR))
#	@$(call extract, CONFIG_MICROBROWSER)
#	@$(call patchin, CONFIG_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_MICROBROWSER_PATH	:= PATH=$(CROSS_PATH)
CONFIG_MICROBROWSER_CONF_TOOL	:= NO
CONFIG_MICROBROWSER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_microbrowser.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_microbrowser.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_microbrowser.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_microbrowser.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_microbrowser)
	@$(call install_fixup, config_microbrowser,PRIORITY,optional)
	@$(call install_fixup, config_microbrowser,SECTION,base)
	@$(call install_fixup, config_microbrowser,AUTHOR,"<elrest>")
	@$(call install_fixup, config_microbrowser,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_microbrowser, 0, 0, 0750, $(CONFIG_MICROBROWSER_DIR)/config_microbrowser, /etc/config-tools/config_microbrowser)

	@$(call install_finish, config_microbrowser)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_microbrowser.clean:
	@$(call targetinfo)
	-cd $(CONFIG_MICROBROWSER_DIR) && rm -f *.o $(CONFIG_MICROBROWSER)	
	@-cd $(CONFIG_MICROBROWSER_DIR) && \
		$(CONFIG_MICROBROWSER_MAKE_ENV) $(CONFIG_MICROBROWSER_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_MICROBROWSER)

# vim: syntax=make
