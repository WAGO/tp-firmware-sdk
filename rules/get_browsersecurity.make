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
PACKAGES-$(PTXCONF_GET_BROWSERSECURITY) += get_browsersecurity

#
# Paths and names
#
GET_BROWSERSECURITY_VERSION	:= 1
GET_BROWSERSECURITY_MD5		:=
GET_BROWSERSECURITY		:= get_browsersecurity
GET_BROWSERSECURITY_URL		:= file://local_src/$(GET_BROWSERSECURITY)
GET_BROWSERSECURITY_DIR		:= $(BUILDDIR)/$(GET_BROWSERSECURITY)
GET_BROWSERSECURITY_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_browsersecurity.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_BROWSERSECURITY_DIR))
#	@$(call extract, GET_BROWSERSECURITY)
#	@$(call patchin, GET_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_BROWSERSECURITY_PATH	:= PATH=$(CROSS_PATH)
GET_BROWSERSECURITY_CONF_TOOL	:= NO
GET_BROWSERSECURITY_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_browsersecurity.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_browsersecurity.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_browsersecurity.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_BROWSERSECURITY)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_browsersecurity.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_browsersecurity)
	@$(call install_fixup, get_browsersecurity,PRIORITY,optional)
	@$(call install_fixup, get_browsersecurity,SECTION,base)
	@$(call install_fixup, get_browsersecurity,AUTHOR,"<elrest>")
	@$(call install_fixup, get_browsersecurity,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_browsersecurity, 0, 0, 0755, $(GET_BROWSERSECURITY_DIR)/get_browsersecurity, /etc/config-tools/get_browsersecurity)

	@$(call install_finish, get_browsersecurity)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_browsersecurity.clean:
	@$(call targetinfo)
	@-cd $(GET_BROWSERSECURITY_DIR) && \
		$(GET_BROWSERSECURITY_ENV) $(GET_BROWSERSECURITY_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_BROWSERSECURITY)

# vim: syntax=make
