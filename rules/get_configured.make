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
PACKAGES-$(PTXCONF_GET_CONFIGURED) += get_configured

#
# Paths and names
#
GET_CONFIGURED_VERSION	:= 1
GET_CONFIGURED_MD5		:=
GET_CONFIGURED		:= get_configured
GET_CONFIGURED_URL		:= file://local_src/$(GET_CONFIGURED)
GET_CONFIGURED_DIR		:= $(BUILDDIR)/$(GET_CONFIGURED)
GET_CONFIGURED_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_configured.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_CONFIGURED_DIR))
#	@$(call extract, GET_CONFIGURED)
#	@$(call patchin, GET_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_CONFIGURED_PATH	:= PATH=$(CROSS_PATH)
GET_CONFIGURED_CONF_TOOL	:= NO
GET_CONFIGURED_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_configured.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_configured.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_configured.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_CONFIGURED)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_configured.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_configured)
	@$(call install_fixup, get_configured,PRIORITY,optional)
	@$(call install_fixup, get_configured,SECTION,base)
	@$(call install_fixup, get_configured,AUTHOR,"<elrest>")
	@$(call install_fixup, get_configured,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_configured, 0, 0, 0755, $(GET_CONFIGURED_DIR)/get_configured, /etc/config-tools/get_configured)

	@$(call install_finish, get_configured)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_configured.clean:
	@$(call targetinfo)
	cd $(GET_CONFIGURED_DIR) && rm -f *.o $(GET_CONFIGURED)	
	@-cd $(GET_CONFIGURED_DIR) && \
		$(GET_CONFIGURED_MAKE_ENV) $(GET_CONFIGURED_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_CONFIGURED)

# vim: syntax=make
