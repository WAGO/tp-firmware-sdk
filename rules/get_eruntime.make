# -*-makefile-*-
#
# Copyright (C) 2018 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_GET_ERUNTIME) += get_eruntime

#
# Paths and names
#
GET_ERUNTIME_VERSION	:= 1
GET_ERUNTIME_MD5		:=
GET_ERUNTIME		:= get_eruntime
GET_ERUNTIME_URL		:= file://local_src/$(GET_ERUNTIME)
GET_ERUNTIME_DIR		:= $(BUILDDIR)/$(GET_ERUNTIME)
GET_ERUNTIME_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_eruntime.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_ERUNTIME_DIR))
#	@$(call extract, GET_ERUNTIME)
#	@$(call patchin, GET_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_ERUNTIME_PATH	:= PATH=$(CROSS_PATH)
GET_ERUNTIME_CONF_TOOL	:= NO
GET_ERUNTIME_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_eruntime.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_eruntime.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_eruntime.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_ERUNTIME)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_eruntime.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_eruntime)
	@$(call install_fixup, get_eruntime,PRIORITY,optional)
	@$(call install_fixup, get_eruntime,SECTION,base)
	@$(call install_fixup, get_eruntime,AUTHOR,"<elrest>")
	@$(call install_fixup, get_eruntime,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_eruntime, 0, 0, 0755, $(GET_ERUNTIME_DIR)/get_eruntime, /etc/config-tools/get_eruntime)

	@$(call install_finish, get_eruntime)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_eruntime.clean:
	@$(call targetinfo)
	-cd $(GET_ERUNTIME_DIR) && rm -f *.o $(GET_ERUNTIME)	
	@-cd $(GET_ERUNTIME_DIR) && \
		$(GET_ERUNTIME_MAKE_ENV) $(GET_ERUNTIME_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_ERUNTIME)

# vim: syntax=make
