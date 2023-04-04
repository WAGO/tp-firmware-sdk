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
PACKAGES-$(PTXCONF_GET_GESTURE) += get_gesture

#
# Paths and names
#
GET_GESTURE_VERSION	:= 1
GET_GESTURE_MD5		:=
GET_GESTURE		:= get_gesture
GET_GESTURE_URL		:= file://local_src/$(GET_GESTURE)
GET_GESTURE_DIR		:= $(BUILDDIR)/$(GET_GESTURE)
GET_GESTURE_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_gesture.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_GESTURE_DIR))
#	@$(call extract, GET_GESTURE)
#	@$(call patchin, GET_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_GESTURE_PATH	:= PATH=$(CROSS_PATH)
GET_GESTURE_CONF_TOOL	:= NO
GET_GESTURE_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_gesture.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_gesture.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_gesture.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_GESTURE)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_gesture.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_gesture)
	@$(call install_fixup, get_gesture,PRIORITY,optional)
	@$(call install_fixup, get_gesture,SECTION,base)
	@$(call install_fixup, get_gesture,AUTHOR,"elrest")
	@$(call install_fixup, get_gesture,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_gesture, 0, 0, 0750, $(GET_GESTURE_DIR)/get_gesture, /etc/config-tools/get_gesture)

	@$(call install_finish, get_gesture)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_gesture.clean:
	@$(call targetinfo)
	-cd $(GET_GESTURE_DIR) && rm -f *.o $(GET_GESTURE)	
	@-cd $(GET_GESTURE_DIR) && \
		$(GET_GESTURE_MAKE_ENV) $(GET_GESTURE_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_GESTURE)

# vim: syntax=make
