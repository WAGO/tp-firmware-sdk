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
PACKAGES-$(PTXCONF_GET_PLCSELECT) += get_plcselect

#
# Paths and names
#
GET_PLCSELECT_VERSION	:= 1
GET_PLCSELECT_MD5		:=
GET_PLCSELECT		:= get_plcselect
GET_PLCSELECT_URL		:= file://local_src/$(GET_PLCSELECT)
GET_PLCSELECT_DIR		:= $(BUILDDIR)/$(GET_PLCSELECT)
GET_PLCSELECT_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_plcselect.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_PLCSELECT_DIR))
#	@$(call extract, GET_PLCSELECT)
#	@$(call patchin, GET_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_PLCSELECT_PATH	:= PATH=$(CROSS_PATH)
GET_PLCSELECT_CONF_TOOL	:= NO
GET_PLCSELECT_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_plcselect.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_plcselect.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_plcselect.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_PLCSELECT)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_plcselect.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_plcselect)
	@$(call install_fixup, get_plcselect,PRIORITY,optional)
	@$(call install_fixup, get_plcselect,SECTION,base)
	@$(call install_fixup, get_plcselect,AUTHOR,"<elrest>")
	@$(call install_fixup, get_plcselect,DESCRIPTION,missing)

	@$(call install_copy, get_plcselect, 0, 0, 0755, /etc/config-tools);

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_plcselect, 0, 0, 0755, $(GET_PLCSELECT_DIR)/get_plcselect, /etc/config-tools/get_plcselect)

	@$(call install_finish, get_plcselect)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_plcselect.clean:
	@$(call targetinfo)
	cd $(GET_PLCSELECT_DIR) && rm -f *.o $(GET_PLCSELECT)
	@-cd $(GET_PLCSELECT_DIR) && \
		$(GET_PLCSELECT_MAKE_ENV) $(GET_PLCSELECT_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_PLCSELECT)

# vim: syntax=make
