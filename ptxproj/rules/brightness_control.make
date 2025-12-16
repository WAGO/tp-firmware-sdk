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
PACKAGES-$(PTXCONF_BRIGHTNESS_CONTROL) += brightness_control

#
# Paths and names
#
BRIGHTNESS_CONTROL_VERSION	:= 1.0.1
BRIGHTNESS_CONTROL_MD5		:=
BRIGHTNESS_CONTROL		:= brightness_control
BRIGHTNESS_CONTROL_URL		:= file://local_src/$(BRIGHTNESS_CONTROL)
BRIGHTNESS_CONTROL_DIR		:= $(BUILDDIR)/$(BRIGHTNESS_CONTROL)
BRIGHTNESS_CONTROL_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/brightness_control.extract:
#	@$(call targetinfo)
#	@$(call clean, $(BRIGHTNESS_CONTROL_DIR))
#	@$(call extract, BRIGHTNESS_CONTROL)
#	@$(call patchin, BRIGHTNESS_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#BRIGHTNESS_CONTROL_PATH	:= PATH=$(CROSS_PATH)
BRIGHTNESS_CONTROL_CONF_TOOL	:= NO
BRIGHTNESS_CONTROL_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/brightness_control.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, BRIGHTNESS_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/brightness_control.compile:
#	@$(call targetinfo)
#	@$(call world/compile, BRIGHTNESS_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/brightness_control.install:
#	@$(call targetinfo)
#	@$(call world/install, BRIGHTNESS_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/brightness_control.targetinstall:
	@$(call targetinfo)

	@$(call install_init, brightness_control)
	@$(call install_fixup, brightness_control,PRIORITY,optional)
	@$(call install_fixup, brightness_control,SECTION,base)
	@$(call install_fixup, brightness_control,AUTHOR,"<elrest>")
	@$(call install_fixup, brightness_control,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, brightness_control, 0, 0, 0750, $(BRIGHTNESS_CONTROL_DIR)/brightness_control, /usr/bin/brightness_control)

	@$(call install_finish, brightness_control)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/brightness_control.clean:
	@$(call targetinfo)
	@-cd $(BRIGHTNESS_CONTROL_DIR) && \
		$(BRIGHTNESS_CONTROL_MAKE_ENV) $(BRIGHTNESS_CONTROL_PATH) $(MAKE) clean
	@$(call clean_pkg, BRIGHTNESS_CONTROL)

# vim: syntax=make
