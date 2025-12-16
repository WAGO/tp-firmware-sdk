# -*-makefile-*-
#
# Copyright (C) 2017 by <elrest>
#
# See CREDITS for details about who has contributed to this project.
#
# For further information about the PTXdist project and license conditions
# see the README file.
#

#
# We provide this package
#
PACKAGES-$(PTXCONF_XDOTOOL) += xdotool

#
# Paths and names
#
XDOTOOL_VERSION	:= 3.20160805.1
XDOTOOL_SUFFIX	:= tar.xz
XDOTOOL_MD5		:= 38b9dc9f1c80a7c60cdbd290bd1b8a94
XDOTOOL			:= xdotool-$(XDOTOOL_VERSION)
XDOTOOL_URL		:= https://svsv01003/svn/repo14/$(XDOTOOL).$(XDOTOOL_SUFFIX)
XDOTOOL_SOURCE	:= $(SRCDIR)/$(XDOTOOL).$(XDOTOOL_SUFFIX)
XDOTOOL_DIR		:= $(BUILDDIR)/$(XDOTOOL)
XDOTOOL_LICENSE	:= unknown
XDOTOOL_DEVPKG  := NO

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/xdotool.extract:
#	@$(call targetinfo)
#	@$(call clean, $(XDOTOOL_DIR))
#	@$(call extract, XDOTOOL)
#	@$(call patchin, XDOTOOL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#XDOTOOL_PATH	:= PATH=$(CROSS_PATH)
XDOTOOL_CONF_TOOL	:= NO
XDOTOOL_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/xdotool.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, XDOTOOL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

$(STATEDIR)/xdotool.compile:
#	@$(call targetinfo)
#	@$(call world/compile, XDOTOOL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/xdotool.install:
#	@$(call targetinfo)
#	@$(call world/install, XDOTOOL)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/xdotool.targetinstall:
	@$(call targetinfo)

	@$(call install_init, xdotool)
	@$(call install_fixup, xdotool,PRIORITY,optional)
	@$(call install_fixup, xdotool,SECTION,base)
	@$(call install_fixup, xdotool,AUTHOR,"<elrest>")
	@$(call install_fixup, xdotool,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, xdotool, 0, 0, 0755, $(XDOTOOL_DIR)/xdotool, /usr/bin/xdotool)
	@$(call install_copy, xdotool, 0, 0, 0644, $(XDOTOOL_DIR)/libxdo.so, /usr/lib/libxdo.so)
	@$(call install_link, xdotool, libxdo.so, /usr/lib/libxdo.so.3)

	@$(call install_finish, xdotool)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/xdotool.clean:
	@$(call targetinfo)
	@-cd $(XDOTOOL_DIR) && \
		$(XDOTOOL_ENV) $(XDOTOOL_PATH) $(MAKE) clean
	@$(call clean_pkg, XDOTOOL)

# vim: syntax=make
