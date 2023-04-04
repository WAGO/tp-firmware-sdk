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
PACKAGES-$(PTXCONF_CONFIG_TESTABILITY) += config_testability

#
# Paths and names
#
CONFIG_TESTABILITY_VERSION	:= 1
CONFIG_TESTABILITY_MD5		:=
CONFIG_TESTABILITY		:= config_testability
CONFIG_TESTABILITY_URL		:= file://local_src/$(CONFIG_TESTABILITY)
CONFIG_TESTABILITY_DIR		:= $(BUILDDIR)/$(CONFIG_TESTABILITY)
CONFIG_TESTABILITY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_testability.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_TESTABILITY_DIR))
#	@$(call extract, CONFIG_TESTABILITY)
#	@$(call patchin, CONFIG_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_TESTABILITY_PATH	:= PATH=$(CROSS_PATH)
CONFIG_TESTABILITY_CONF_TOOL	:= NO
CONFIG_TESTABILITY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_testability.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_testability.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_testability.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_TESTABILITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_testability.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_testability)
	@$(call install_fixup, config_testability,PRIORITY,optional)
	@$(call install_fixup, config_testability,SECTION,base)
	@$(call install_fixup, config_testability,AUTHOR,"<elrest>")
	@$(call install_fixup, config_testability,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_testability, 0, 0, 0750, $(CONFIG_TESTABILITY_DIR)/config_testability, /etc/config-tools/config_testability)

	@$(call install_finish, config_testability)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_testability.clean:
	@$(call targetinfo)
	-cd $(CONFIG_TESTABILITY_DIR) && rm -f *.o $(CONFIG_TESTABILITY)	
	@-cd $(CONFIG_TESTABILITY_DIR) && \
		$(CONFIG_TESTABILITY_ENV) $(CONFIG_TESTABILITY_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_TESTABILITY)

# vim: syntax=make
