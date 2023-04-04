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
PACKAGES-$(PTXCONF_GET_TOUCHBEEPER) += get_touchbeeper

#
# Paths and names
#
GET_TOUCHBEEPER_VERSION	:= 0.0.1
GET_TOUCHBEEPER_MD5		:=
GET_TOUCHBEEPER		:= get_touchbeeper
GET_TOUCHBEEPER_URL		:= file://local_src/$(GET_TOUCHBEEPER)
GET_TOUCHBEEPER_DIR		:= $(BUILDDIR)/$(GET_TOUCHBEEPER)
GET_TOUCHBEEPER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_touchbeeper.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_TOUCHBEEPER_DIR))
#	@$(call extract, GET_TOUCHBEEPER)
#	@$(call patchin, GET_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_TOUCHBEEPER_PATH	:= PATH=$(CROSS_PATH)
GET_TOUCHBEEPER_CONF_TOOL	:= NO
GET_TOUCHBEEPER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_touchbeeper.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_touchbeeper.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_touchbeeper.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_TOUCHBEEPER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_touchbeeper.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_touchbeeper)
	@$(call install_fixup, get_touchbeeper,PRIORITY,optional)
	@$(call install_fixup, get_touchbeeper,SECTION,base)
	@$(call install_fixup, get_touchbeeper,AUTHOR,"<elrest>")
	@$(call install_fixup, get_touchbeeper,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_touchbeeper, 0, 0, 0750, $(GET_TOUCHBEEPER_DIR)/get_touchbeeper, /etc/config-tools/get_touchbeeper)

	@$(call install_finish, get_touchbeeper)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_touchbeeper.clean:
	@$(call targetinfo)
	-cd $(GET_TOUCHBEEPER_DIR) && rm -f *.o $(GET_TOUCHBEEPER)	
	@-cd $(GET_TOUCHBEEPER_DIR) && \
		$(GET_TOUCHBEEPER_MAKE_ENV) $(GET_TOUCHBEEPER_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_TOUCHBEEPER)

# vim: syntax=make
