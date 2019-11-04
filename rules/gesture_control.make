# -*-makefile-*-
#
# Copyright (C) 2017 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GESTURE_CONTROL) += gesture_control

#
# Paths and names
#
GESTURE_CONTROL_VERSION	:= 1
GESTURE_CONTROL_MD5		:=
GESTURE_CONTROL		:= gesture_control
GESTURE_CONTROL_URL		:= file://local_src/$(GESTURE_CONTROL)
GESTURE_CONTROL_DIR		:= $(BUILDDIR)/$(GESTURE_CONTROL)
GESTURE_CONTROL_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GESTURE_CONTROL_DIR))
#	@$(call extract, GESTURE_CONTROL)
#	@$(call patchin, GESTURE_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GESTURE_CONTROL_PATH	:= PATH=$(CROSS_PATH)
GESTURE_CONTROL_CONF_TOOL	:= NO
GESTURE_CONTROL_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/gesture_control.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GESTURE_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GESTURE_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control.install:
#	@$(call targetinfo)
#	@$(call world/install, GESTURE_CONTROL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gesture_control.targetinstall:
	@$(call targetinfo)

	@$(call install_init, gesture_control)
	@$(call install_fixup, gesture_control,PRIORITY,optional)
	@$(call install_fixup, gesture_control,SECTION,base)
	@$(call install_fixup, gesture_control,AUTHOR,"<elrest>")
	@$(call install_fixup, gesture_control,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, gesture_control, 0, 0, 0755, $(GESTURE_CONTROL_DIR)/gesture_control, /usr/bin/gesture_control)

	@$(call install_finish, gesture_control)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/gesture_control.clean:
	@$(call targetinfo)
	@-cd $(GESTURE_CONTROL_DIR) && \
		$(GESTURE_CONTROL_ENV) $(GESTURE_CONTROL_PATH) $(MAKE) clean
	@$(call clean_pkg, GESTURE_CONTROL)

# vim: syntax=make
