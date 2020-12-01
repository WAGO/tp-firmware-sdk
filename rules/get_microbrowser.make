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
PACKAGES-$(PTXCONF_GET_MICROBROWSER) += get_microbrowser

#
# Paths and names
#
GET_MICROBROWSER_VERSION	:= 1
GET_MICROBROWSER_MD5		:=
GET_MICROBROWSER		:= get_microbrowser
GET_MICROBROWSER_URL		:= file://local_src/$(GET_MICROBROWSER)
GET_MICROBROWSER_DIR		:= $(BUILDDIR)/$(GET_MICROBROWSER)
GET_MICROBROWSER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_microbrowser.extract:
#	@$(call targetinfo)
#	@$(call clean, $(GET_MICROBROWSER_DIR))
#	@$(call extract, GET_MICROBROWSER)
#	@$(call patchin, GET_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#GET_MICROBROWSER_PATH	:= PATH=$(CROSS_PATH)
GET_MICROBROWSER_CONF_TOOL	:= NO
GET_MICROBROWSER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/get_microbrowser.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, GET_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_microbrowser.compile:
#	@$(call targetinfo)
#	@$(call world/compile, GET_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/get_microbrowser.install:
#	@$(call targetinfo)
#	@$(call world/install, GET_MICROBROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/get_microbrowser.targetinstall:
	@$(call targetinfo)

	@$(call install_init, get_microbrowser)
	@$(call install_fixup, get_microbrowser,PRIORITY,optional)
	@$(call install_fixup, get_microbrowser,SECTION,base)
	@$(call install_fixup, get_microbrowser,AUTHOR,"<elrest>")
	@$(call install_fixup, get_microbrowser,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, get_microbrowser, 0, 0, 0755, $(GET_MICROBROWSER_DIR)/get_microbrowser, /etc/config-tools/get_microbrowser)

	@$(call install_finish, get_microbrowser)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/get_microbrowser.clean:
	@$(call targetinfo)
	cd $(GET_MICROBROWSER_DIR) && rm -f *.o $(GET_MICROBROWSER)	
	@-cd $(GET_MICROBROWSER_DIR) && \
		$(GET_MICROBROWSER_MAKE_ENV) $(GET_MICROBROWSER_PATH) $(MAKE) clean
	@$(call clean_pkg, GET_MICROBROWSER)

# vim: syntax=make
