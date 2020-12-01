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
PACKAGES-$(PTXCONF_GET_DISPLAY) += get_display

#
# Paths and names
#
GET_DISPLAY_VERSION	:= 1
GET_DISPLAY_MD5		:=
GET_DISPLAY		:= get_display
GET_DISPLAY_URL		:= file://local_src/$(GET_DISPLAY)
GET_DISPLAY_DIR		:= $(BUILDDIR)/$(GET_DISPLAY)
GET_DISPLAY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_display.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_DISPLAY_DIR))
#	@$(call extract, GET_DISPLAY)
#	@$(call patchin, GET_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_DISPLAY_PATH	:= PATH=$(CROSS_PATH)
GET_DISPLAY_CONF_TOOL	:= NO
GET_DISPLAY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_display.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_display.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_display.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_DISPLAY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_display.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_display)
	@$(call install_fixup, get_display,PRIORITY,optional)
	@$(call install_fixup, get_display,SECTION,base)
	@$(call install_fixup, get_display,AUTHOR,"<elrest>")
	@$(call install_fixup, get_display,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_display, 0, 0, 0755, $(GET_DISPLAY_DIR)/get_display, /etc/config-tools/get_display)

	@$(call install_finish, get_display)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_display.clean:
	@$(call targetinfo)
	cd $(GET_DISPLAY_DIR) && rm -f *.o $(GET_DISPLAY)
	@-cd $(GET_DISPLAY_DIR) && \
		$(GET_DISPLAY_MAKE_ENV) $(GET_DISPLAY_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_DISPLAY)

# vim: syntax=make
