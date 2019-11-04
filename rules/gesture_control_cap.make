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
PACKAGES-$(PTXCONF_GESTURE_CONTROL_CAP) += gesture_control_cap

#
# Paths and names
#
GESTURE_CONTROL_CAP_VERSION	:= 1
GESTURE_CONTROL_CAP_MD5		:=
GESTURE_CONTROL_CAP		:= gesture_control_cap
GESTURE_CONTROL_CAP_URL		:= file://local_src/$(GESTURE_CONTROL_CAP)
GESTURE_CONTROL_CAP_DIR		:= $(BUILDDIR)/$(GESTURE_CONTROL_CAP)
GESTURE_CONTROL_CAP_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control_cap.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GESTURE_CONTROL_CAP_DIR))
#	@$(call extract, GESTURE_CONTROL_CAP)
#	@$(call patchin, GESTURE_CONTROL_CAP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GESTURE_CONTROL_CAP_PATH	:= PATH=$(CROSS_PATH)
GESTURE_CONTROL_CAP_CONF_TOOL	:= NO
GESTURE_CONTROL_CAP_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/gesture_control_cap.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GESTURE_CONTROL_CAP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control_cap.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GESTURE_CONTROL_CAP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/gesture_control_cap.install:
#	@$(call targetinfo)
#	@$(call world/install, GESTURE_CONTROL_CAP)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/gesture_control_cap.targetinstall:
	@$(call targetinfo)

	@$(call install_init, gesture_control_cap)
	@$(call install_fixup, gesture_control_cap,PRIORITY,optional)
	@$(call install_fixup, gesture_control_cap,SECTION,base)
	@$(call install_fixup, gesture_control_cap,AUTHOR,"<elrest>")
	@$(call install_fixup, gesture_control_cap,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, gesture_control_cap, 0, 0, 0755, $(GESTURE_CONTROL_CAP_DIR)/gesture_control_cap, /usr/bin/gesture_control_cap)

	@$(call install_finish, gesture_control_cap)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/gesture_control_cap.clean:
	@$(call targetinfo)
	@-cd $(GESTURE_CONTROL_CAP_DIR) && \
		$(GESTURE_CONTROL_CAP_ENV) $(GESTURE_CONTROL_CAP_PATH) $(MAKE) clean
	@$(call clean_pkg, GESTURE_CONTROL_CAP)

# vim: syntax=make
