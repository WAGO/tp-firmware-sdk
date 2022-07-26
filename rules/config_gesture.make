# -*-makefile-*-
#
# Copyright (C) 2014 by elrest
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_CONFIG_GESTURE) += config_gesture

#
# Paths and names
#
CONFIG_GESTURE_VERSION	:= 1
CONFIG_GESTURE_MD5		:=
CONFIG_GESTURE		:= config_gesture
CONFIG_GESTURE_URL		:= file://local_src/$(CONFIG_GESTURE)
CONFIG_GESTURE_DIR		:= $(BUILDDIR)/$(CONFIG_GESTURE)
CONFIG_GESTURE_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_gesture.extract:
#	@$(call targetinfo)
#	@$(call clean, $(CONFIG_GESTURE_DIR))
#	@$(call extract, CONFIG_GESTURE)
#	@$(call patchin, CONFIG_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#CONFIG_GESTURE_PATH	:= PATH=$(CROSS_PATH)
CONFIG_GESTURE_CONF_TOOL	:= NO
CONFIG_GESTURE_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/config_gesture.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, CONFIG_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_gesture.compile:
#	@$(call targetinfo)
#	@$(call world/compile, CONFIG_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/config_gesture.install:
#	@$(call targetinfo)
#	@$(call world/install, CONFIG_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/config_gesture.targetinstall:
	@$(call targetinfo)

	@$(call install_init, config_gesture)
	@$(call install_fixup, config_gesture,PRIORITY,optional)
	@$(call install_fixup, config_gesture,SECTION,base)
	@$(call install_fixup, config_gesture,AUTHOR,"elrest")
	@$(call install_fixup, config_gesture,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, config_gesture, 0, 0, 0755, $(CONFIG_GESTURE_DIR)/config_gesture, /etc/config-tools/config_gesture)

	@$(call install_finish, config_gesture)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/config_gesture.clean:
	@$(call targetinfo)
	-cd $(CONFIG_GESTURE_DIR) && rm -f *.o $(CONFIG_GESTURE)	
	@-cd $(CONFIG_GESTURE_DIR) && \
		$(CONFIG_GESTURE_MAKE_ENV) $(CONFIG_GESTURE_PATH) $(MAKE) clean
	@$(call clean_pkg, CONFIG_GESTURE)

# vim: syntax=make
