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
PACKAGES-$(PTXCONF_CONFIG_BOOT) += config_boot

#
# Paths and names
#
CONFIG_BOOT_VERSION	:= 1
CONFIG_BOOT_MD5		:=
CONFIG_BOOT		:= config_boot
CONFIG_BOOT_URL		:= file://local_src/$(CONFIG_BOOT)
CONFIG_BOOT_DIR		:= $(BUILDDIR)/$(CONFIG_BOOT)
CONFIG_BOOT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_boot.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_BOOT_DIR))
#	@$(call extract, CONFIG_BOOT)
#	@$(call patchin, CONFIG_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_BOOT_PATH	:= PATH=$(CROSS_PATH)
CONFIG_BOOT_CONF_TOOL	:= NO
CONFIG_BOOT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_boot.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_boot.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_boot.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_BOOT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_boot.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_boot)
	@$(call install_fixup, config_boot,PRIORITY,optional)
	@$(call install_fixup, config_boot,SECTION,base)
	@$(call install_fixup, config_boot,AUTHOR,"<elrest>")
	@$(call install_fixup, config_boot,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_boot, 0, 0, 0750, $(CONFIG_BOOT_DIR)/config_boot, /etc/config-tools/config_boot)

	@$(call install_finish, config_boot)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_boot.clean:
	@$(call targetinfo)
	-cd $(CONFIG_BOOT_DIR) && rm -f *.o $(CONFIG_BOOT)	
	@-cd $(CONFIG_BOOT_DIR) && \
		$(CONFIG_BOOT_MAKE_ENV) $(CONFIG_BOOT_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_BOOT)

# vim: syntax=make
