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
PACKAGES-$(PTXCONF_START_BROWSER) += start_browser

#
# Paths and names
#
START_BROWSER_VERSION	:= 1
START_BROWSER_MD5		:=
START_BROWSER		:= start_browser
START_BROWSER_URL		:= file://local_src/$(START_BROWSER)
START_BROWSER_DIR		:= $(BUILDDIR)/$(START_BROWSER)
START_BROWSER_LICENSE	:= unknown

# ----------------------------------------------------------------------------
# Extract
# ----------------------------------------------------------------------------

#$(STATEDIR)/start_browser.extract:
#	@$(call targetinfo)
#	@$(call clean, $(START_BROWSER_DIR))
#	@$(call extract, START_BROWSER)
#	@$(call patchin, START_BROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Prepare
# ----------------------------------------------------------------------------

#START_BROWSER_PATH	:= PATH=$(CROSS_PATH)
START_BROWSER_CONF_TOOL	:= NO
START_BROWSER_MAKE_ENV	:= $(CROSS_ENV)

#$(STATEDIR)/start_browser.prepare:
#	@$(call targetinfo)
#	@$(call world/prepare, START_BROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------------

#$(STATEDIR)/start_browser.compile:
#	@$(call targetinfo)
#	@$(call world/compile, START_BROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------

#$(STATEDIR)/start_browser.install:
#	@$(call targetinfo)
#	@$(call world/install, START_BROWSER)
#	@$(call touch)

# ----------------------------------------------------------------------------
# Target-Install
# ----------------------------------------------------------------------------

$(STATEDIR)/start_browser.targetinstall:
	@$(call targetinfo)

	@$(call install_init, start_browser)
	@$(call install_fixup, start_browser,PRIORITY,optional)
	@$(call install_fixup, start_browser,SECTION,base)
	@$(call install_fixup, start_browser,AUTHOR,"<elrest>")
	@$(call install_fixup, start_browser,DESCRIPTION,missing)

#
# TODO: Add here all files that should be copied to the target
# Note: Add everything before(!) call to macro install_finish
#
	@$(call install_copy, start_browser, 0, 0, 0755, $(START_BROWSER_DIR)/start_browser, /etc/config-tools/start_browser)

	@$(call install_finish, start_browser)

	@$(call touch)

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------

$(STATEDIR)/start_browser.clean:
	@$(call targetinfo)
	cd $(START_BROWSER_DIR) && rm -f *.o $(START_BROWSER)	
	@-cd $(START_BROWSER_DIR) && \
		$(START_BROWSER_MAKE_ENV) $(START_BROWSER_PATH) $(MAKE) clean
	@$(call clean_pkg, START_BROWSER)

# vim: syntax=make
