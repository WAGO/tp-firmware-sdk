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
PACKAGES-$(PTXCONF_GET_BACKLIGHT) += get_backlight

#
# Paths and names
#
GET_BACKLIGHT_VERSION	:= 0.0.1
GET_BACKLIGHT_MD5		:=
GET_BACKLIGHT		:= get_backlight
GET_BACKLIGHT_URL		:= file://local_src/$(GET_BACKLIGHT)
GET_BACKLIGHT_DIR		:= $(BUILDDIR)/$(GET_BACKLIGHT)
GET_BACKLIGHT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_backlight.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_BACKLIGHT_DIR))
#	@$(call extract, GET_BACKLIGHT)
#	@$(call patchin, GET_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_BACKLIGHT_PATH	:= PATH=$(CROSS_PATH)
GET_BACKLIGHT_CONF_TOOL	:= NO
GET_BACKLIGHT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_backlight.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_backlight.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_backlight.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_BACKLIGHT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_backlight.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_backlight)
	@$(call install_fixup, get_backlight,PRIORITY,optional)
	@$(call install_fixup, get_backlight,SECTION,base)
	@$(call install_fixup, get_backlight,AUTHOR,"<elrest>")
	@$(call install_fixup, get_backlight,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_backlight, 0, 0, 0755, $(GET_BACKLIGHT_DIR)/get_backlight, /etc/config-tools/get_backlight)

	@$(call install_finish, get_backlight)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_backlight.clean:
	@$(call targetinfo)
	-cd $(GET_BACKLIGHT_DIR) && rm -f *.o $(GET_BACKLIGHT)	
	@-cd $(GET_BACKLIGHT_DIR) && \
		$(GET_BACKLIGHT_MAKE_ENV) $(GET_BACKLIGHT_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_BACKLIGHT)

# vim: syntax=make
