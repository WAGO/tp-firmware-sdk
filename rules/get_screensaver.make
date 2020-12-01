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
PACKAGES-$(PTXCONF_GET_SCREENSAVER) += get_screensaver

#
# Paths and names
#
GET_SCREENSAVER_VERSION	:= 0.0.1
GET_SCREENSAVER_MD5		:=
GET_SCREENSAVER		:= get_screensaver
GET_SCREENSAVER_URL		:= file://local_src/$(GET_SCREENSAVER)
GET_SCREENSAVER_DIR		:= $(BUILDDIR)/$(GET_SCREENSAVER)
GET_SCREENSAVER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_screensaver.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_SCREENSAVER_DIR))
#	@$(call extract, GET_SCREENSAVER)
#	@$(call patchin, GET_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_SCREENSAVER_PATH	:= PATH=$(CROSS_PATH)
GET_SCREENSAVER_CONF_TOOL	:= NO
GET_SCREENSAVER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_screensaver.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_screensaver.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_screensaver.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_SCREENSAVER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_screensaver.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_screensaver)
	@$(call install_fixup, get_screensaver,PRIORITY,optional)
	@$(call install_fixup, get_screensaver,SECTION,base)
	@$(call install_fixup, get_screensaver,AUTHOR,"<elrest>")
	@$(call install_fixup, get_screensaver,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_screensaver, 0, 0, 0755, $(GET_SCREENSAVER_DIR)/get_screensaver, /etc/config-tools/get_screensaver)

	@$(call install_finish, get_screensaver)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_screensaver.clean:
	@$(call targetinfo)
	cd $(GET_SCREENSAVER_DIR) && rm -f *.o $(GET_SCREENSAVER)	
	@-cd $(GET_SCREENSAVER_DIR) && \
		$(GET_SCREENSAVER_MAKE_ENV) $(GET_SCREENSAVER_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_SCREENSAVER)

# vim: syntax=make
