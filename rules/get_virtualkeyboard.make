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
PACKAGES-$(PTXCONF_GET_VIRTUALKEYBOARD) += get_virtualkeyboard

#
# Paths and names
#
GET_VIRTUALKEYBOARD_VERSION	:= 1
GET_VIRTUALKEYBOARD_MD5		:=
GET_VIRTUALKEYBOARD		:= get_virtualkeyboard
GET_VIRTUALKEYBOARD_URL		:= file://local_src/$(GET_VIRTUALKEYBOARD)
GET_VIRTUALKEYBOARD_DIR		:= $(BUILDDIR)/$(GET_VIRTUALKEYBOARD)
GET_VIRTUALKEYBOARD_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_virtualkeyboard.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_VIRTUALKEYBOARD_DIR))
#	@$(call extract, GET_VIRTUALKEYBOARD)
#	@$(call patchin, GET_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_VIRTUALKEYBOARD_PATH	:= PATH=$(CROSS_PATH)
GET_VIRTUALKEYBOARD_CONF_TOOL	:= NO
GET_VIRTUALKEYBOARD_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_virtualkeyboard.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_virtualkeyboard.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_virtualkeyboard.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_VIRTUALKEYBOARD)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_virtualkeyboard.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_virtualkeyboard)
	@$(call install_fixup, get_virtualkeyboard,PRIORITY,optional)
	@$(call install_fixup, get_virtualkeyboard,SECTION,base)
	@$(call install_fixup, get_virtualkeyboard,AUTHOR,"<elrest>")
	@$(call install_fixup, get_virtualkeyboard,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_virtualkeyboard, 0, 0, 0755, $(GET_VIRTUALKEYBOARD_DIR)/get_virtualkeyboard, /etc/config-tools/get_virtualkeyboard)

	@$(call install_finish, get_virtualkeyboard)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_virtualkeyboard.clean:
	@$(call targetinfo)
	cd $(GET_VIRTUALKEYBOARD_DIR) && rm -f *.o $(GET_VIRTUALKEYBOARD)	
	@-cd $(GET_VIRTUALKEYBOARD_DIR) && \
		$(GET_VIRTUALKEYBOARD_ENV) $(GET_VIRTUALKEYBOARD_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_VIRTUALKEYBOARD)

# vim: syntax=make
