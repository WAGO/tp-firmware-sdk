# -*-makefile-*-
#
# Copyright (C) 2015 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GET_FONTS) += get_fonts

#
# Paths and names
#
GET_FONTS_VERSION	:= 1
GET_FONTS_MD5		:=
GET_FONTS		:= get_fonts
GET_FONTS_URL		:= file://local_src/$(GET_FONTS)
GET_FONTS_DIR		:= $(BUILDDIR)/$(GET_FONTS)
GET_FONTS_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_fonts.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_FONTS_DIR))
#	@$(call extract, GET_FONTS)
#	@$(call patchin, GET_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_FONTS_PATH	:= PATH=$(CROSS_PATH)
GET_FONTS_CONF_TOOL	:= NO
GET_FONTS_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_fonts.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_fonts.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_fonts.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_FONTS)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_fonts.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_fonts)
	@$(call install_fixup, get_fonts,PRIORITY,optional)
	@$(call install_fixup, get_fonts,SECTION,base)
	@$(call install_fixup, get_fonts,AUTHOR,"<elrest>")
	@$(call install_fixup, get_fonts,DESCRIPTION,missing)

	@$(call install_copy, get_fonts, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_fonts, 0, 0, 0755, $(GET_FONTS_DIR)/get_fonts, /etc/config-tools/get_fonts)

	@$(call install_finish, get_fonts)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_fonts.clean:
	@$(call targetinfo)
	@-cd $(GET_FONTS_DIR) && \
		$(GET_FONTS_ENV) $(GET_FONTS_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_FONTS)

# vim: syntax=make
