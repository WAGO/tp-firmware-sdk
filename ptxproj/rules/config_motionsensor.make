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
PACKAGES-$(PTXCONF_CONFIG_MOTIONSENSOR) += config_motionsensor

#
# Paths and names
#
CONFIG_MOTIONSENSOR_VERSION	:= 0.0.1
CONFIG_MOTIONSENSOR_MD5		:=
CONFIG_MOTIONSENSOR		:= config_motionsensor
CONFIG_MOTIONSENSOR_URL		:= file://local_src/$(CONFIG_MOTIONSENSOR)
CONFIG_MOTIONSENSOR_DIR		:= $(BUILDDIR)/$(CONFIG_MOTIONSENSOR)
CONFIG_MOTIONSENSOR_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_motionsensor.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_MOTIONSENSOR_DIR))
#	@$(call extract, CONFIG_MOTIONSENSOR)
#	@$(call patchin, CONFIG_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_MOTIONSENSOR_PATH	:= PATH=$(CROSS_PATH)
CONFIG_MOTIONSENSOR_CONF_TOOL	:= NO
CONFIG_MOTIONSENSOR_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_motionsensor.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_motionsensor.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_motionsensor.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_MOTIONSENSOR)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_motionsensor.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_motionsensor)
	@$(call install_fixup, config_motionsensor,PRIORITY,optional)
	@$(call install_fixup, config_motionsensor,SECTION,base)
	@$(call install_fixup, config_motionsensor,AUTHOR,"<elrest>")
	@$(call install_fixup, config_motionsensor,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_motionsensor, 0, 0, 0750, $(CONFIG_MOTIONSENSOR_DIR)/config_motionsensor, /etc/config-tools/config_motionsensor)

	@$(call install_finish, config_motionsensor)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_motionsensor.clean:
	@$(call targetinfo)
	-cd $(CONFIG_MOTIONSENSOR_DIR) && rm -f *.o $(CONFIG_MOTIONSENSOR)	
	@-cd $(CONFIG_MOTIONSENSOR_DIR) && \
		$(CONFIG_MOTIONSENSOR_MAKE_ENV) $(CONFIG_MOTIONSENSOR_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_MOTIONSENSOR)

# vim: syntax=make
